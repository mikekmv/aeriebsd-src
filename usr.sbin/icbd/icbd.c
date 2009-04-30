/*
 * Copyright (c) 2009 Mike Belopuhov
 * Copyright (c) 2007 Oleg Safiullin
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef lint
static const char rcsid[] = "$ABSD: icbd.c,v 1.7 2009/04/29 17:06:46 mikeb Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <locale.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>
#include <login_cap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <unistd.h>

#include "icb.h"
#include "icbd.h"

extern char *__progname;

int creategroups = 0;

static int foreground;
static int verbose;

void usage(void);
void getpeerinfo(struct icb_session *, char *, size_t, int *);
void icbd_accept(int, short, void *);
void icbd_drop(struct icb_session *, char *);
void icbd_error(struct bufferevent *, short, void *);
void icbd_dispatch(struct bufferevent *, void *);
void icbd_log(struct icb_session *, int, char *);
void icbd_grplist(char *);
void icbd_restrict(void);
void icbd_write(struct icb_session *, char *, size_t);

int
main(int argc, char *argv[])
{
	struct icbd_callbacks ic = { icbd_drop, icbd_log, icbd_write };
	const char *cause = NULL;
	int ch, nsocks = 0, save_errno = 0;
	int inet4 = 0, inet6 = 0;

	/* init group lists before calling icb_addgroup */
	icb_init(&ic);

	while ((ch = getopt(argc, argv, "46CdG:v")) != -1)
		switch (ch) {
		case '4':
			inet4++;
			break;
		case '6':
			inet6++;
			break;
		case 'C':
			creategroups++;
			break;
		case 'd':
			foreground++;
			break;
		case 'G':
			icbd_grplist(optarg);
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	/* XXX: add group "1" by default */
	if (!creategroups)
		icb_addgroup(NULL, "1", NULL);

	if (argc == 0)
		argc++;

	if (inet4 && inet6)
		errx(EX_CONFIG, "Can't specify both -4 and -6");

	tzset();
	setlocale(LC_ALL, "C");

	if (foreground)
		openlog("icbd", LOG_PID | LOG_PERROR, LOG_DAEMON);
	else
		openlog("icbd", LOG_PID | LOG_NDELAY, LOG_DAEMON);

	if (!foreground && daemon(0, 0) < 0)
		err(EX_OSERR, NULL);

	(void)event_init();

	for (ch = 0; ch < argc; ch++) {
		struct addrinfo hints, *res, *res0;
		struct event *ev;
		char *addr, *port;
		int error, s, on = 1;

		addr = port = NULL;
		if (argv[ch] != NULL) {
			if (argv[ch][0] != ':')
				addr = argv[ch];
			if ((port = strrchr(argv[ch], ':')) != NULL)
				*port++ = '\0';
		}

		bzero(&hints, sizeof(hints));
		if (inet4 || inet6)
			hints.ai_family = inet4 ? PF_INET : PF_INET6;
		else
			hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		if ((error = getaddrinfo(addr, port ? port : "icb", &hints,
		    &res0)) != 0) {
			syslog(LOG_ERR, "%s", gai_strerror(error));
			return (EX_UNAVAILABLE);
		}

		for (res = res0; res != NULL; res = res->ai_next) {
			if ((s = socket(res->ai_family, res->ai_socktype,
			    res->ai_protocol)) < 0) {
				cause = "socket";
				save_errno = errno;
				continue;
			}

			if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on,
			    sizeof(on)) < 0) {
				cause = "SO_REUSEADDR";
				save_errno = errno;
				(void)close(s);
				continue;
			}

			if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
				cause = "bind";
				save_errno = errno;
				(void)close(s);
				continue;
			}

			(void)listen(s, TCP_BACKLOG);

			if ((ev = calloc(1, sizeof(*ev))) == NULL)
				err(EX_UNAVAILABLE, NULL);
			event_set(ev, s, EV_READ | EV_PERSIST, icbd_accept, ev);
			if (event_add(ev, NULL) < 0) {
				syslog(LOG_ERR, "event_add: %m");
				return (EX_UNAVAILABLE);
			}

			nsocks++;
		}

		freeaddrinfo(res0);
	}

	if (nsocks == 0) {
		errno = save_errno;
		syslog(LOG_ERR, "%s: %m", cause);
		return (EX_UNAVAILABLE);
	}

	if (!foreground)
		icbd_restrict();

	signal(SIGPIPE, SIG_IGN);

	(void)event_dispatch();

	syslog(LOG_ERR, "event_dispatch: %m");

	return (EX_UNAVAILABLE);
}

void
icbd_accept(int fd, short event __attribute__((__unused__)),
    void *arg __attribute__((__unused__)))
{
	char host[MAXHOSTNAMELEN];
	struct bufferevent **bev = NULL;
	struct sockaddr_storage ss;
	struct icb_session *is;
	socklen_t ss_len = sizeof(ss);
	int s;

	ss.ss_len = ss_len;
	if ((s = accept(fd, (struct sockaddr *)&ss, &ss_len)) < 0) {
		syslog(LOG_ERR, "accept: %m");
		return;
	}
	if ((is = calloc(1, sizeof(*is))) == NULL) {
		syslog(LOG_ERR, "calloc: %m");
		(void)close(s);
		return;
	}
	bev = GETBEVP(is);
	if ((*bev = bufferevent_new(s, icbd_dispatch, NULL, icbd_error,
	    is)) == NULL) {
		syslog(LOG_ERR, "bufferevent_new: %m");
		(void)close(s);
		free(is);
		return;
	}
	if (bufferevent_enable(*bev, EV_READ)) {
		syslog(LOG_ERR, "bufferevent_enable: %m");
		(void)close(s);
		bufferevent_free(*bev);
		free(is);
		return;
	}
	icbd_log(is, ICB_LOG_NORMAL, "connected");

	/* save host information */
	getpeerinfo(is, host, sizeof(host), NULL);
	if (strlen(host) > 0)
		strlcpy(is->host, host, sizeof(is->host));

	/* start icb conversation */
	icb_start(is);
}

__dead void
usage(void)
{
	(void)fprintf(stderr, "usage: %s [-46Cdv] [-G group1[,group2,...]] "
	   "[[addr][:port] ...]\n",  __progname);
	exit(EX_USAGE);
}

/*
 *  bufferevent functions
 */

void
icbd_dispatch(struct bufferevent *bev, void *arg)
{
	char buf[ICB_MSGSIZE + 1];
	struct icb_session *is = (struct icb_session *)arg;
	size_t len;

	bzero(buf, sizeof(buf));
	len = bufferevent_read(bev, buf, sizeof(buf)-1);
	if (len <= 0) {
		icbd_drop(is, "short read");
		return;
	}
	icb_input(is, buf, len);
}

void
icbd_write(struct icb_session *is, char *buf, size_t size)
{
	if (bufferevent_write(GETBEV(is), buf, size) == -1)
		syslog(LOG_ERR, "bufferevent_write: %m");
}

void
icbd_drop(struct icb_session *is, char *reason)
{
	struct bufferevent **bev = NULL;
	char *msg = "disconnected";

	if (reason && strlen(reason) > 0)
		msg = reason;
	icb_remove(is, msg);
	icbd_log(is, ICB_LOG_NORMAL, msg);
	bev = GETBEVP(is);
	evbuffer_write((*bev)->output, EVBUFFER_FD(*bev));
	(void)close(EVBUFFER_FD(*bev));
	bufferevent_free(*bev);
	free(is);
}

void
icbd_error(struct bufferevent *bev __attribute__((__unused__)), short what,
    void *arg)
{
	struct icb_session *is = (struct icb_session *)arg;

	if (what & EVBUFFER_TIMEOUT)
		icbd_drop(is, "timeout");
	else if (what & EVBUFFER_EOF)
		icbd_drop(is, "client hung up");
	else if (what & EVBUFFER_ERROR)
		icbd_drop(is, (what & EVBUFFER_READ) ? "read error" :
		    "write error");	
}

void
icbd_log(struct icb_session *is, int level, char *msg)
{
	char addr[INET6_ADDRSTRLEN];
	int port;

	if (!verbose && level == ICB_LOG_DEBUG)
		return;

	if (level == ICB_LOG_ERROR)
		level = LOG_ERR;
	else if (level == ICB_LOG_NORMAL)
		level = LOG_NOTICE;
	else
		level = LOG_DEBUG;
	if (is) {
		getpeerinfo(is, addr, sizeof(addr), &port);
		syslog(level, "%s:%u: %s", addr, port, msg);
	} else {
		syslog(level, "%s", msg);
	}
}

void
icbd_restrict(void)
{
	struct passwd *pw;

	if ((pw = getpwnam(ICBD_USER)) == NULL) {
		syslog(LOG_ERR, "No passwd entry for %s", ICBD_USER);
		exit(EX_NOUSER);
	}
	if (setusercontext(NULL, pw, pw->pw_uid,
	    LOGIN_SETALL & ~LOGIN_SETUSER) < 0) {
		syslog(LOG_ERR, "%s:%m", pw->pw_name);
		exit(EX_NOPERM);
	}
	if (chroot(pw->pw_dir) < 0) {
		syslog(LOG_ERR, "%s:%m", pw->pw_dir);
		exit(EX_UNAVAILABLE);
	}
	if (setuid(pw->pw_uid) < 0) {
		syslog(LOG_ERR, "%d:%m", pw->pw_uid);
		exit(EX_NOPERM);
	}
	if (chdir("/") < 0) {
		syslog(LOG_ERR, "%m");
		exit(EX_UNAVAILABLE);
	}
	(void)setproctitle("icbd");
	endpwent();
}

void
icbd_grplist(char *list)
{
	char *s, *s1, *s2;
	int last = 0;

	if (!list || strlen(list) == 0)
		return;

	/* "group1[:pass1][,group2[:pass2],...]" */
	s = list;
	s1 = s2 = NULL;
	while (!last && s) {
		if ((s1 = strchr(s, ',')) != NULL)
			*s1 = '\0';
		else {
			last = 1;
			s1 = s;
		}
		if ((s2 = strchr(s, ':')) != NULL)
			*s2 = '\0';
		icb_addgroup(NULL, s, s2 ? ++s2 : NULL);
		s = ++s1;
		s1 = s2 = NULL;
	}
}

time_t
getmonotime(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		syslog(LOG_ERR, "%m");
		exit(EX_OSERR);
	}	
	return (ts.tv_sec);
}

void
getpeerinfo(struct icb_session *is, char *addrbuf, size_t addrsz, int *port)
{
	char addr[INET6_ADDRSTRLEN];
	struct sockaddr_storage ss;
	socklen_t ss_len = sizeof(ss);

	bzero(addr, sizeof(addr));
	bzero(&ss, sizeof(ss));
	if (getpeername(EVBUFFER_FD(GETBEV(is)), (struct sockaddr *)&ss,
	    &ss_len) != 0)
		return;
	switch (ss.ss_family) {
	case AF_INET:
		(void)inet_ntop(AF_INET,
		    &((struct sockaddr_in *)&ss)->sin_addr, addr,
		    sizeof(addr));
		if (port)
			*port = ntohs(((struct sockaddr_in *)&ss)->sin_port);
		break;
	case AF_INET6:
		(void)inet_ntop(AF_INET6,
		    &((struct sockaddr_in6 *)&ss)->sin6_addr, addr,
		    sizeof(addr));
		if (port)
			*port = ntohs(((struct sockaddr_in6 *)&ss)->sin6_port);
		break;
	}
	strlcpy(addrbuf, addr, addrsz);
}
