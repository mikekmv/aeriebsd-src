
/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <pwd.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "ntpd.h"

void		sighdlr(int);
__dead void	usage(void);
int		main(int, char *[]);
int		check_child(pid_t, const char *);
int		dispatch_imsg(struct ntpd_conf *);
void		reset_adjtime(void);
int		ntpd_adjtime(int64_t);
void		ntpd_adjfreq(int64_t, int);
void		ntpd_settime(struct timeval *);
void		readfreq(void);
int		writefreq(int64_t);

volatile sig_atomic_t	 quit = 0;
volatile sig_atomic_t	 reconfig = 0;
volatile sig_atomic_t	 sigchld = 0;
struct imsgbuf		*ibuf;
int			 debugsyslog = 0;
int			 timeout = INFTIM;

void
sighdlr(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		quit = 1;
		break;
	case SIGCHLD:
		sigchld = 1;
		break;
	case SIGHUP:
		reconfig = 1;
		break;
	}
}

__dead void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-46dnSsv] [-f file]\n", __progname);
	exit(1);
}

#define POLL_MAX		8
#define PFD_PIPE		0

int
main(int argc, char *argv[])
{
	struct ntpd_conf	 lconf;
	struct pollfd		 pfd[POLL_MAX];
	pid_t			 chld_pid = 0, pid;
	const char		*conffile;
	int			 ch, nfds;
	int			 pipe_chld[2];
	struct passwd		*pw;

	conffile = CONFFILE;

	bzero(&lconf, sizeof(lconf));

	log_init(1);		/* log to stderr until daemonized */
	res_init();		/* XXX */

	lconf.family = PF_UNSPEC;
	while ((ch = getopt(argc, argv, "46df:nsSv")) != -1) {
		switch (ch) {
		case '4':
			lconf.family = PF_INET;
			break;
		case '6':
			lconf.family = PF_INET6;
			break;
		case 'd':
			lconf.debug = 1;
			break;
		case 'f':
			conffile = optarg;
			break;
		case 'n':
			lconf.noaction = 1;
			break;
		case 's':
			lconf.settime = 1;
			break;
		case 'S':
			lconf.settime = 0;
			break;
		case 'v':
			debugsyslog = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();

	if (parse_config(conffile, &lconf))
		exit(1);

	if (lconf.noaction) {
		fprintf(stderr, "configuration OK\n");
		exit(0);
	}

	if (geteuid())
		errx(1, "need root privileges");

	if ((pw = getpwnam(NTPD_USER)) == NULL)
		errx(1, "unknown user %s", NTPD_USER);

	endpwent();

	reset_adjtime();
	if (!lconf.settime) {
		log_init(lconf.debug);
		if (!lconf.debug)
			if (daemon(1, 0))
				fatal("daemon");
	} else
		timeout = SETTIME_TIMEOUT * 1000;

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_chld) == -1)
		fatal("socketpair");

	signal(SIGCHLD, sighdlr);
	/* fork child process */
	chld_pid = ntp_main(pipe_chld, &lconf, pw);

	setproctitle("[priv]");
	readfreq();

	signal(SIGTERM, sighdlr);
	signal(SIGINT, sighdlr);
	signal(SIGHUP, sighdlr);

	close(pipe_chld[1]);

	if ((ibuf = malloc(sizeof(struct imsgbuf))) == NULL)
		fatal(NULL);
	imsg_init(ibuf, pipe_chld[0]);

	while (quit == 0) {
		pfd[PFD_PIPE].fd = ibuf->fd;
		pfd[PFD_PIPE].events = POLLIN;
		if (ibuf->w.queued)
			pfd[PFD_PIPE].events |= POLLOUT;

		if ((nfds = poll(pfd, 1, timeout)) == -1)
			if (errno != EINTR) {
				log_warn("poll error");
				quit = 1;
			}

		if (nfds == 0 && lconf.settime) {
			lconf.settime = 0;
			timeout = INFTIM;
			log_init(lconf.debug);
			log_debug("no reply received in time, skipping initial "
			    "time setting");
			if (!lconf.debug)
				if (daemon(1, 0))
					fatal("daemon");
		}

		if (nfds > 0 && (pfd[PFD_PIPE].revents & POLLOUT))
			if (msgbuf_write(&ibuf->w) < 0) {
				log_warn("pipe write error (to child)");
				quit = 1;
			}

		if (nfds > 0 && pfd[PFD_PIPE].revents & POLLIN) {
			nfds--;
			if (dispatch_imsg(&lconf) == -1)
				quit = 1;
		}

		if (sigchld) {
			if (check_child(chld_pid, "child")) {
				quit = 1;
				chld_pid = 0;
			}
			sigchld = 0;
		}

	}

	signal(SIGCHLD, SIG_DFL);

	if (chld_pid)
		kill(chld_pid, SIGTERM);

	do {
		if ((pid = wait(NULL)) == -1 &&
		    errno != EINTR && errno != ECHILD)
			fatal("wait");
	} while (pid != -1 || (pid == -1 && errno == EINTR));

	msgbuf_clear(&ibuf->w);
	free(ibuf);
	log_info("Terminating");
	return (0);
}

int
check_child(pid_t pid, const char *pname)
{
	int	 status, sig;
	char	*signame;

	if (waitpid(pid, &status, WNOHANG) > 0) {
		if (WIFEXITED(status)) {
			log_warnx("Lost child: %s exited", pname);
			return (1);
		}
		if (WIFSIGNALED(status)) {
			sig = WTERMSIG(status);
			signame = strsignal(sig) ? strsignal(sig) : "unknown";
			log_warnx("Lost child: %s terminated; signal %d (%s)",
			    pname, sig, signame);
			return (1);
		}
	}

	return (0);
}

int
dispatch_imsg(struct ntpd_conf *lconf)
{
	struct imsg		 imsg;
	int			 n, cnt;
	struct timeval		 tv;
	char			*name;
	struct ntp_addr		*h, *hn;
	struct buf		*buf;
	int64_t			 d;

	if ((n = imsg_read(ibuf)) == -1)
		return (-1);

	if (n == 0) {	/* connection closed */
		log_warnx("dispatch_imsg in main: pipe closed");
		return (-1);
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			return (-1);

		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_ADJTIME:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(d))
				fatalx("invalid IMSG_ADJTIME received");
			memcpy(&d, imsg.data, sizeof(d));
			n = ntpd_adjtime(d);
			imsg_compose(ibuf, IMSG_ADJTIME, 0, 0, &n, sizeof(n));
			break;
		case IMSG_ADJFREQ:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(d))
				fatalx("invalid IMSG_ADJFREQ received");
			memcpy(&d, imsg.data, sizeof(d));
			ntpd_adjfreq(d, 1);
			break;
		case IMSG_SETTIME:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(tv))
				fatalx("invalid IMSG_SETTIME received");
			if (!lconf->settime)
				break;
			log_init(lconf->debug);
			memcpy(&tv, imsg.data, sizeof(tv));
			ntpd_settime(&tv);
			/* daemonize now */
			if (!lconf->debug)
				if (daemon(1, 0))
					fatal("daemon");
			lconf->settime = 0;
			timeout = INFTIM;
			break;
		case IMSG_HOST_DNS:
			name = imsg.data;
			if (imsg.hdr.len < 1 + IMSG_HEADER_SIZE)
				fatalx("invalid IMSG_HOST_DNS received");
			imsg.hdr.len -= 1 + IMSG_HEADER_SIZE;
			if (name[imsg.hdr.len] != '\0' ||
			    strlen(name) != imsg.hdr.len)
				fatalx("invalid IMSG_HOST_DNS received");
			if ((cnt = host_dns(name, &hn, lconf)) == -1)
				break;
			buf = imsg_create(ibuf, IMSG_HOST_DNS,
			    imsg.hdr.peerid, 0,
			    cnt * sizeof(struct sockaddr_storage));
			if (buf == NULL)
				break;
			if (cnt > 0)
				for (h = hn; h != NULL; h = h->next)
					imsg_add(buf, &h->ss, sizeof(h->ss));

			imsg_close(ibuf, buf);
			break;
		default:
			break;
		}
		imsg_free(&imsg);
	}
	return (0);
}

void
reset_adjtime(void)
{
	struct timeval	tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (adjtime(&tv, NULL) == -1)
		log_warn("reset adjtime failed");
}

int
ntpd_adjtime(int64_t d)
{
	static int	firstadj = 1;
	struct timeval	tv, tv1, olddelta;
	char		*s = "";
	int		synced = 0;

	getoffset(&olddelta);
	int642timeval(d, &tv);
	timeradd(&tv, &olddelta, &tv);

	if (d < 0) {
		s = "-";
		d = -d;
	}

	int642timeval(d, &tv1);
	if (tv1.tv_sec || tv1.tv_usec >= LOG_NEGLIGEE * 1000)
		log_info("adjusting local clock by %s%ld.%06lus",
		    s, tv1.tv_sec, tv1.tv_usec);
	else
		log_debug("adjusting local clock by %s%ld.%06lus",
		    s, tv1.tv_sec, tv1.tv_usec);

	if (adjtime(&tv, &olddelta) == -1)
		log_warn("adjtime failed");
	else if (!firstadj && olddelta.tv_sec == 0 && olddelta.tv_usec == 0)
		synced = 1;
	firstadj = 0;
	return (synced);
}

void
ntpd_adjfreq(int64_t relfreq, int wrlog)
{
	int64_t curfreq;
	int r;

	if (adjfreq(NULL, &curfreq) == -1) {
		log_warn("adjfreq failed");
		return;
	}
	curfreq += relfreq;
	if (adjfreq(&curfreq, NULL) == -1)
		log_warn("adjfreq failed");

	/*
	 * adjfreq's unit is ns/s shifted left 32.
	 * We log values in part per million.
	 */
	r = writefreq(curfreq);
	if (wrlog) {
		char *s1 = "", *s2 = "";

		if (relfreq < 0) {
			relfreq = -relfreq;
			s1 = "-";
		}
		if (curfreq < 0) {
			curfreq = -curfreq;
			s2 = "-";
		}
		log_info("adjusting clock frequency by %s0.%06d to "
		    "%s%d.%06dppm%s",
		    s1, (int)(relfreq >> 32) % 1000000,
		    s2, (int)(curfreq >> 32) / 1000000,
		    (int)(curfreq >> 32) % 1000000,
		    r ? " (no drift file)" : "");
	}
}

void
ntpd_settime(struct timeval *tv)
{
	struct timeval	curtime;
	char		buf[80];
	time_t		tval;

	if (gettimeofday(&curtime, NULL) == -1) {
		log_warn("gettimeofday");
		return;
	}
	curtime.tv_usec += tv->tv_usec + 1000000;
	curtime.tv_sec += tv->tv_sec - 1 + (curtime.tv_usec / 1000000);
	curtime.tv_usec %= 1000000;

	if (settimeofday(&curtime, NULL) == -1) {
		log_warn("settimeofday");
		return;
	}
	tval = curtime.tv_sec;
	strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Z %Y",
	    localtime(&tval));
	log_info("set local clock to %s (offset %ld.%06lus)", buf,
	    tv->tv_sec, tv->tv_usec);
}

void
readfreq(void)
{
	int64_t current, d;
	FILE *fp;

	fp = fopen(DRIFTFILE, "r");
	if (fp == NULL) {
		/* if the drift file has been deleted by the user, reset */
		current = 0;
		if (adjfreq(&current, NULL) == -1)
			log_warn("adjfreq reset failed");
		return;
	}

	/* if we're adjusting frequency already, don't override */
	if (adjfreq(NULL, &current) == -1)
		log_warn("adjfreq failed");
	else if (current == 0) {
		if (fscanf(fp, "%lld", &d) == 1)
			ntpd_adjfreq(d, 0);
		else
			log_warnx("can't read %s", DRIFTFILE);
	}
	fclose(fp);
}

int
writefreq(int64_t d)
{
	int r;
	FILE *fp;
	static int warnonce = 1;

	fp = fopen(DRIFTFILE, "w");
	if (fp == NULL) {
		if (warnonce) {
			log_warn("can't open %s", DRIFTFILE);
			warnonce = 0;
		}
		return -1;
	}

	fprintf(fp, "%lld\n", d);
	r = ferror(fp);
	if (fclose(fp) != 0 || r != 0) {
		if (warnonce) {
			log_warnx("can't write %s", DRIFTFILE);
			warnonce = 0;
		}
		unlink(DRIFTFILE);
		return -1;
	}
	return 0;
}

int
ntp_recvmsg(int fd, struct sockaddr *ss, char *buf, struct ntp_msg *msg,
    int64_t *t)
{
	struct msghdr	somsg;
	struct timeval  tv1, tv2;
	struct iovec	iov[1];
	union {
		struct cmsghdr hdr;
		char buf[CMSG_SPACE(sizeof tv1)];
	} cmsgbuf;
	struct cmsghdr	*cmsg;
	ssize_t	size;

	somsg.msg_name = ss;
	somsg.msg_namelen = sizeof(struct sockaddr_storage);
	somsg.msg_iov = iov;
	iov[0].iov_base = buf;
	iov[0].iov_len = NTP_MSGSIZE;
	somsg.msg_iovlen = 1;
	somsg.msg_control = cmsgbuf.buf;
	somsg.msg_controllen = sizeof cmsgbuf.buf;
	somsg.msg_flags = 0;

	getoffset(&tv2);
	if ((size = recvmsg(fd, &somsg, 0)) < 0) {
		if (errno == EHOSTUNREACH || errno == EHOSTDOWN ||
		    errno == ENETUNREACH || errno == ENETDOWN ||
		    errno == ECONNREFUSED || errno == EADDRNOTAVAIL ||
		    errno == ENOPROTOOPT || errno == ENOENT) {
			ntp_log_error(ss, "recvmsg");
			return -1;
		} else
			fatal("recvmsg");
	}

	if (somsg.msg_flags & MSG_TRUNC) {
		errno = EINVAL;
		ntp_log_error(ss, "recvmsg packet");
		return -1;
	}

	if (somsg.msg_flags & MSG_CTRUNC) {
		errno = EINVAL;
		ntp_log_error(ss, "recvmsg control data");
		return -1;
	}

	*t = 0;
	for (cmsg = CMSG_FIRSTHDR(&somsg); cmsg != NULL;
	    cmsg = CMSG_NXTHDR(&somsg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET &&
		    cmsg->cmsg_type == SCM_TIMESTAMP) {
			memcpy(&tv1, CMSG_DATA(cmsg), sizeof tv1);
			*t = timeval2int64(&tv1);
			*t += (int64_t)JAN_1970 << 31;
			*t += timeval2int64(&tv2);
			break;
		}
	}

	if (*t == 0) {
		errno = EINVAL;
		ntp_log_error(ss, "recvmsg control format");
		return -1;
	}

	return ntp_getmsg(ss, buf, size, msg);
}

void
ntp_log_error(struct sockaddr *sa, const char *msg)
{
	const char *address;

	address = log_sockaddr(sa);
	log_warn("%s %s", msg, address);
}
