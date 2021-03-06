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
static const char rcsid[] = "$ABSD: icbd.c,v 1.23 2010/01/10 12:45:24 kmerz Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <pwd.h>
#include <login_cap.h>
#include <locale.h>
#include <netdb.h>
#include <event.h>
#include <errno.h>
#include <err.h>

#include "icb.h"
#include "icbd.h"

extern char *__progname;

int creategroups;
int foreground;
int verbose;

void usage(void);
void getpeerinfo(struct icb_session *);
void icbd_accept(int, short, void *);
void icbd_drop(struct icb_session *, char *);
void icbd_ioerr(struct bufferevent *, short, void *);
void icbd_dispatch(struct bufferevent *, void *);
void icbd_log(struct icb_session *, int, const char *, ...);
void icbd_grplist(char *);
void icbd_restrict(void);
void icbd_write(struct icb_session *, char *, ssize_t);

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

	/* add group "1" as it's a login group for most of the clients */
	if (icb_addgroup(NULL, "1", NULL) == NULL)
		err(EX_UNAVAILABLE, NULL);

	if (argc == 0)
		argc++;

	if (inet4 && inet6)
		errx(EX_CONFIG, "Can't specify both -4 and -6");

	tzset();
	(void)setlocale(LC_ALL, "C");

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

		bzero(&hints, sizeof hints);
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
			    sizeof on) < 0) {
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

			if ((ev = calloc(1, sizeof *ev)) == NULL)
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

	/* start a dns resolver thread */
	icbd_dns_init();

	if (!foreground)
		icbd_restrict();

	(void)signal(SIGPIPE, SIG_IGN);

	(void)event_dispatch();

	syslog(LOG_ERR, "event_dispatch: %m");

	return (EX_UNAVAILABLE);
}

void
icbd_dns(int fd, short event, void *arg)
{
	struct icb_session *is = arg;

	if (event != EV_READ)
		return;

	if (verbose)
		syslog(LOG_DEBUG, "icbd_dns");

	if (read(fd, is->host, sizeof is->host) < 0)
		syslog(LOG_ERR, "read: %m");

	is->host[sizeof is->host - 1] = '\0';

	if (verbose)
		syslog(LOG_DEBUG, "icbd_dns: resolved %s", is->host);
}

void
icbd_accept(int fd, short event, void *arg)
{
	struct sockaddr_storage ss;
	struct icb_session *is;
	socklen_t ss_len = sizeof ss;
	int s;

	ss.ss_len = ss_len;
	if ((s = accept(fd, (struct sockaddr *)&ss, &ss_len)) < 0) {
		syslog(LOG_ERR, "accept: %m");
		return;
	}
	if ((is = calloc(1, sizeof *is)) == NULL) {
		syslog(LOG_ERR, "calloc: %m");
		(void)close(s);
		return;
	}
	if ((is->bev = bufferevent_new(s, icbd_dispatch, NULL, icbd_ioerr,
	    is)) == NULL) {
		syslog(LOG_ERR, "bufferevent_new: %m");
		(void)close(s);
		free(is);
		return;
	}
	if (bufferevent_enable(is->bev, EV_READ)) {
		syslog(LOG_ERR, "bufferevent_enable: %m");
		(void)close(s);
		bufferevent_free(is->bev);
		free(is);
		return;
	}

	/* save host information */
	getpeerinfo(is);

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
	struct icb_session *is = (struct icb_session *)arg;

	if (is->length == 0) {
		bzero(is->buffer, sizeof is->buffer);
		/* read length */
		(void)bufferevent_read(bev, is->buffer, 1);
		/* we're about to read the whole packet */
		is->length = (size_t)(unsigned char)is->buffer[0];
		if (is->length == 0) {
			icbd_drop(is, "invalid packet");
			return;
		}
		if (EVBUFFER_LENGTH(EVBUFFER_INPUT(bev)) < is->length) {
			/* set watermark to the expected length */
			bufferevent_setwatermark(bev, EV_READ, is->length, 0);
			return;
		}
	}
	(void)bufferevent_read(bev, &is->buffer[1], is->length);
#ifdef DEBUG
	{
		int i;

		printf("-> read from %s:%d:\n", is->host, is->port);
		for (i = 0; i < (int)is->length + 1; i++)
			printf(" %02x", (unsigned char)is->buffer[i]);
		printf("\n");
	}
#endif
	icb_input(is);
	is->length = 0;
}

void
icbd_write(struct icb_session *is, char *buf, ssize_t size)
{
	if (bufferevent_write(is->bev, buf, size) == -1)
		syslog(LOG_ERR, "bufferevent_write: %m");
#ifdef DEBUG
	{
		int i;

		printf("-> wrote to %s:%d:\n", is->host, is->port);
		for (i = 0; i < size; i++)
			printf(" %02x", (unsigned char)buf[i]);
		printf("\n");
	}
#endif
}

void
icbd_drop(struct icb_session *is, char *reason)
{
	if (reason) {
		icb_remove(is, reason);
		icbd_log(is, LOG_DEBUG, reason);
	} else
		icb_remove(is, NULL);
	(void)evbuffer_write(EVBUFFER_OUTPUT(is->bev), EVBUFFER_FD(is->bev));
	(void)close(EVBUFFER_FD(is->bev));
	bufferevent_free(is->bev);
	free(is);
}

void
icbd_ioerr(struct bufferevent *bev, short what, void *arg)
{
	struct icb_session *is = (struct icb_session *)arg;

	if (what & EVBUFFER_TIMEOUT)
		icbd_drop(is, "timeout");
	else if (what & EVBUFFER_EOF)
		icbd_drop(is, NULL);
	else if (what & EVBUFFER_ERROR)
		icbd_drop(is, (what & EVBUFFER_READ) ? "read error" :
		    "write error");
}

void
icbd_log(struct icb_session *is, int level, const char *fmt, ...)
{
	char buf[512];
	va_list ap;

	if (!verbose && level == LOG_DEBUG)
		return;

	va_start(ap, fmt);
	(void)vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	if (is)
		syslog(level, "%s:%u: %s", is->host, is->port, buf);
	else
		syslog(level, "%s", buf);
}

void
icbd_restrict(void)
{
	struct stat	sb;
	struct passwd	*pw;

	if ((pw = getpwnam(ICBD_USER)) == NULL) {
		syslog(LOG_ERR, "No passwd entry for %s", ICBD_USER);
		exit(EX_NOUSER);
	}

	if (setusercontext(NULL, pw, pw->pw_uid,
	    LOGIN_SETALL & ~LOGIN_SETUSER) < 0) {
		syslog(LOG_ERR, "%s: %m", pw->pw_name);
		exit(EX_NOPERM);
	}

	if (stat(pw->pw_dir, &sb) == -1) {
		syslog(LOG_ERR, "%s: %m", pw->pw_name);
		exit(EX_NOPERM);
	}

	if (sb.st_uid != 0 || (sb.st_mode & (S_IWGRP|S_IWOTH)) != 0) {
		syslog(LOG_ERR, "bad directory permissions");
		exit(EX_NOPERM);
	}

	if (chroot(pw->pw_dir) < 0) {
		syslog(LOG_ERR, "%s: %m", pw->pw_dir);
		exit(EX_UNAVAILABLE);
	}

	if (setuid(pw->pw_uid) < 0) {
		syslog(LOG_ERR, "%d: %m", pw->pw_uid);
		exit(EX_NOPERM);
	}

	if (chdir("/") < 0) {
		syslog(LOG_ERR, "/: %m");
		exit(EX_UNAVAILABLE);
	}

	(void)setproctitle("icbd");
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
		if (icb_addgroup(NULL, s, s2 ? ++s2 : NULL) == NULL)
			err(EX_UNAVAILABLE, NULL);
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
getpeerinfo(struct icb_session *is)
{
	struct sockaddr_storage ss;
	socklen_t ss_len = sizeof ss;

	bzero(&ss, sizeof ss);
	if (getpeername(EVBUFFER_FD(is->bev), (struct sockaddr *)&ss,
	    &ss_len) != 0)
		return;

	is->port = 0;
	switch (ss.ss_family) {
	case AF_INET:
		is->port = ntohs(((struct sockaddr_in *)&ss)->sin_port);
		break;

	case AF_INET6:
		is->port = ntohs(((struct sockaddr_in6 *)&ss)->sin6_port);
		break;
	}

	dns_rresolv(is, &ss);
}
