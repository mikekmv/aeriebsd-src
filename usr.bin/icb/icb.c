/*
 * Copyright (c) 2009 Konrad Merz, Mike Belopuhov
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
static const char rcsid[] = "$ABSD$";
#endif /* not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <util.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <term.h>

#include "screen.h"
#include "icb.h"

struct winsize ws;
size_t netlen;
char netbuf[ICB_MSGSIZE+2];
char inputbuf[ICB_MSGSIZE];
char *group, *nick, *server, *port;
int inet4, inet6;
int curspos, from, inputpos, inputlen, netpos, loggedin, sock;

#define	RESET_INPUTBUF() do {			\
		bzero(inputbuf, ICB_MSGSIZE);	\
		inputlen = 0;			\
		inputpos = 0;			\
	} while (0)

static void  usage(void);
void  icb_connect(void);
void  icb_display(const char *, ...);
void  icb_print_edit(void);
void  icb_help(void);
void  icb_input(void);
void  icb_login(void);
void  icb_netin(int, short, void *);
void  icb_output(char *, size_t);
void  icb_send(char *, size_t);
void  icb_sighandler(int);
void  icb_ttycleanup(void);
void  icb_ttyin(int, short, void *);
void  icb_notify(int, short, void *);
void  icb_ttyinit(void);
int   icb_lookupcmd(char *);
char *icb_parse(char *, size_t, size_t *);

int
main(int argc, char **argv)
{
	char *p = NULL;
	char g[] = "1";
	int ch;

	while ((ch = getopt(argc, argv, "46g:n:")) != -1) {
		switch (ch) {
		case '4':
			inet4++;
			break;
		case '6':
			inet6++;
			break;
		case 'g':
			group = optarg;
			break;
		case 'n':
			nick = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (inet4 && inet6)
		errx(1, "Can't specify both -4 and -6");

	/* group 1 is usually default */ 
	if (group == NULL)
		group = g;

	if (argc < 1)
		usage();
	if ((p = strchr(argv[0], ':')) != NULL) {
		*p = '\0';
		if (*++p)
			port = p;
	}
	server = argv[0];

	if (!nick && (nick = getlogin()) == NULL)
		err(1, NULL);

	tzset();
	setlocale(LC_ALL, "C");
	event_init();

	/* prepare the terminal */	
	icb_ttyinit();

	/* connect to the server */
	icb_connect();

	/* event dispatch loop */
	event_dispatch();

	/* cleanup terminal */
	icb_ttycleanup();

	return (EX_OK);
}

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "%s [-46] [-g group] [-n nick] server[:port]\n",
	    __progname);
	exit(EX_USAGE);
}

void
icb_connect(void)
{
	struct timeval t = { 9.0, 0.0 };
	struct addrinfo hints, *res, *res0;
	struct event *ev, *ev_t;
	const char *cause = NULL;
	int error, save_errno;
	int s = -1, cnt = 0;

	ev = calloc(sizeof(*ev), 1);
	if (!ev)
		err(1, "icb_connect");
	ev_t = calloc(sizeof(*ev_t), 1);
	if (!ev_t)
		err(1, "icb_connect");

	bzero(&hints, sizeof(hints));
	hints.ai_family =
	    inet6 ? PF_INET6 : inet4 ? PF_INET : PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (!port)
		port = "icb";
	error = getaddrinfo(server, port, &hints, &res0);
	if (error)
		errx(1, "%s: %s", server, gai_strerror(error));
	for (res = res0; res; res = res->ai_next, cnt++) {
		s = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (s < 0) {
			cause = "socket";
			continue;
		}
		if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
			cause = "connect";
			save_errno = errno;
			close(s);
			errno = save_errno;
			s = -1;
			continue;
		}
		if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
			cause = "fcntl";
			save_errno = errno;
			close(s);
			errno = save_errno;
			s = -1;
			continue;
		}
		break;
	}
	freeaddrinfo(res0);
	if (s < 0)
		err(1, "%s", cause);

	event_set(ev, s, EV_READ | EV_PERSIST, icb_netin, ev);
	if (event_add(ev, NULL) < 0)
		err(1, "event_add");
	sock = s;

	/* 
	 * We send every five seconds a notify to tell the server
	 * we are still alive and avoid a timeout
	 * XXX KM: Doesn't seem to work right now
	 */
 	 event_set(ev_t, -1, EV_PERSIST, icb_notify, ev_t);
	 if (evtimer_add(ev_t, &t))
 		 err(1, "evtimer_add");
}

void
icb_netin(int fd, short event, void *arg)
{
	struct event *ev = (struct event *)arg;
	ssize_t len = 0;

	if ((event & EV_READ) == 0)
		return;

	if (netlen == 0) {
		bzero(netbuf, sizeof netbuf);
		netpos = 0;
		/* read length */
		errno = 0;
		do {
			len = read(fd, netbuf, 1);
		} while (len < 0 && errno == EINTR);
		if (len == 0) {
			icb_display("[=Error=] Server has closed "
			    "connection\n");
			event_del(ev);
			exit(0);
		} else if (len < 0)
			return;
		/* we're about to read the whole packet */
		netlen = (size_t)(unsigned char)netbuf[0];
		netpos++;
	}
reread:
	errno = 0;
	len = read(fd, &netbuf[netpos], netlen);
	if (len < 0 && errno == EAGAIN)
		return;
	else if (len < 0 && errno == EINTR)
		goto reread;
	else if (len == 0) {
		icb_display("[=Error=] Server has closed connection\n");
		event_del(ev);
		return;
	}
	netlen -= len;
	netpos += len;
	if (netlen == 0)
		icb_input();
}

void
icb_send(char *msg, size_t msglen)
{
	write(sock, msg, msglen);
}

/* Initilize tty input */
void
icb_ttyinit(void)
{
	struct event *ev;

	raw_mode(1);
	get_term();
	init();

	signal(SIGINT, icb_sighandler);
	signal(SIGWINCH, icb_sighandler);
	
	ev = calloc(sizeof(*ev), 1);
	if (!ev)
		err(1, "icb_ttyin");

	event_set(ev, STDIN_FILENO, EV_READ | EV_PERSIST, icb_ttyin,
	    NULL);
	if (event_add(ev, NULL) < 0)
		err(1, "icb_ttyinit");
	bzero(&inputbuf, sizeof(inputbuf));
}

void
icb_sighandler(int sig)
{
	struct winsize w;
	int hard;

	switch (sig) {
	case SIGINT:
		RESET_INPUTBUF();
		icb_display("\nDo you really want to quit ? (y/N)");
		if (getchar() == 'y') {
			icb_ttycleanup();
			exit(0);
		}
		del_line();
		break;
	case SIGWINCH:
		/*
		 * Get size of the screen.
		 */
		if (ioctl(2, TIOCGWINSZ, &w) == 0 && w.ws_row > 0)
			sc_height = w.ws_row;
		else
			sc_height = tgetnum("li");
		hard = (sc_height < 0 || tgetflag("hc"));
		if (hard) {
			/* Oh no, this is a hardcopy terminal. */
			sc_height = 24;
		}
		if (ioctl(2, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
			sc_width = w.ws_col;
		else
			sc_width = tgetnum("co");
		if (sc_width < 0)
			sc_width = 80;
		break;
	default:
		break;
	}
}

void
icb_ttycleanup(void)
{
	deinit();
	raw_mode(0);
}

void
icb_ttyin(int fd, short event, void *arg)
{
	char msg[ICB_MSGSIZE];
	char c;
	int action, mlen = 0;

again:
	c = getchar();
	if (c != '\n') {
		action = decode_input(c);
		switch (action) {
		case A_PREFIX:
			goto again;
			break;
		case A_CUR_LEFT:
			if (inputlen > 0) {
				inputpos--;
				curs_left();
			}
			break;
		case A_CUR_RIGHT:
			if (inputpos < inputlen) {
				inputpos++;
				curs_right();
			}
			break;
		case A_BACKSPACE:
			if (inputpos > 0) {
				inputlen--;
				inputpos--;
				memcpy(&inputbuf[inputpos],
				    &inputbuf[inputpos+1],
				    inputlen-inputpos+1);
				icb_print_edit();
			}
			break;
		case A_INVALID:
		default:
			if (inputlen > (ICB_MSGSIZE-1))
				return;
			if (inputpos < inputlen) {
				memcpy(&inputbuf[inputpos],
				    &inputbuf[inputpos-1],
				    inputlen-inputpos+1);
				curs_right();
			}
			inputbuf[inputpos] = c;
			inputlen++;
			inputpos++;

			icb_print_edit();
			break;
		}

		/* To asure that the terminal commands got executeted */
		fflush(stdout);

	} else {
		if (inputlen == 0)
			return;
		
		strlcpy(msg, inputbuf, ICB_MSGSIZE);
		if (inputlen > ICB_MSGSIZE)
			mlen = ICB_MSGSIZE;
		else
			mlen = inputlen;

		/* Clean up and send */
		del_line();
		while((inputlen--) > 0)
			curs_left();
		RESET_INPUTBUF();
		icb_output(msg, mlen);
	}
}

void
icb_display(const char *fmt, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (inputlen != 0) {
		del_line();
		put_line(buf);
		icb_print_edit(); 
	} else {
		put_line(buf);
	}
}

/* Reprint what we editing */
void
icb_print_edit(void)
{
	int mv_cnt = inputpos;
	int len;

	if (inputpos >= sc_width)
		len = inputlen - sc_width; 
	else
		len = 0;
	del_line();
	while((mv_cnt--) >= 0)
		curs_left();
	put_line(&inputbuf[len]);
	mv_cnt = inputlen - inputpos;
	while ((mv_cnt--) > 0)
		curs_left();
}

/* Printing help inside of the icb */
void
icb_help(void)
{
	icb_display("Supported commands are as follows:\n"
	    "    /away            - set away flag\n"
	    "    /boot <nick>     - kick out user <nick>\n"
	    "    /g <group>       - change to group <group>\n"
	    "    /help            - print this help message\n"
	    "    /m <nick> <msg>  - send <nick> private message\n"
	    "    /pass            - pass group moderation\n"
	    "    /quit            - terminate program\n"
	    "    /topic [<topic>] - set or print group topic\n"
	    "    /w               - list users\n");
}

int
icb_lookupcmd(char *msg)
{
	char cmd[16];
	char *p;

	if (msg[0] != '/')
    		return (CMD_NOTCMD);

	/* copy 16 bytes at most */
	strlcpy(cmd, msg, sizeof cmd);
	/* truncate the result at the very first space */
	if ((p = strchr(cmd, ' ')) != NULL)
		*p = '\0';

	if (strcmp(cmd, "/away") == 0 ||
	    strcmp(cmd, "/boot") == 0 ||
	    strcmp(cmd, "/g") == 0 ||
	    strcmp(cmd, "/m") == 0 ||
	    strcmp(cmd, "/noaway") == 0 ||
	    strcmp(cmd, "/pass") == 0 ||
	    strcmp(cmd, "/topic") == 0 ||
	    strcmp(cmd, "/w") == 0)
		return (CMD_ICBCMD);
	if (strncmp(cmd, "/help", 5 ) == 0)
		return (CMD_HELP);
	if (strncmp(cmd, "/quit", 5) == 0)
		return (CMD_QUIT);
	return (CMD_NOTCMD);
}

void
icb_output(char *msg, size_t msglen)
{
	char buf[ICB_MSGSIZE];
	char *print_msg;
	char com[16];
	char *arg;
	int buflen = 1, cmd;

	if (msglen == 0)
		return;

	print_msg = msg;
	cmd = icb_lookupcmd(msg);
	switch (cmd) {
	case CMD_NOTCMD:
		buflen = snprintf(&buf[1], sizeof buf - 1, "%c%s",
		    ICB_M_OPEN, msg);
		break;
	case CMD_QUIT:
		icb_ttycleanup();	
		exit(0);
		/* NOTREACHED */
	case CMD_HELP:
		icb_help();
		return;
	case CMD_ICBCMD:
		msg++;
		/* copy 16 bytes at most */
		strlcpy(com, msg, sizeof com);
		/* truncate the result at the very first space */
		if ((arg = strchr(com, ' ')) != NULL)
			*arg = '\0';
		if ((arg = strchr(msg, ' ')) == NULL) 
			buflen = snprintf(&buf[1], sizeof buf - 1,
			    "%c%s%c", ICB_M_COMMAND, com, ICB_M_SEP);
		else
			buflen = snprintf(&buf[1], sizeof buf - 1,
			    "%c%s%c%s", ICB_M_COMMAND, com,
			    ICB_M_SEP, ++arg);
		break;
	default:
		return;
	}
	buf[0] = ++buflen;
	icb_send(buf, buflen + 1);
	icb_display("%s\n", print_msg);
}

/* ICB protocol functions */
void
icb_login(void)
{
	char buf[ICB_MSGSIZE];
	int buflen;

	buflen = snprintf(&buf[1], sizeof buf - 1,
	    "%c%s%c%s%c%s%clogin%c ",
	    ICB_M_LOGIN, getlogin(), ICB_M_SEP, nick, ICB_M_SEP, group,
	    ICB_M_SEP, ICB_M_SEP);
	buf[0] = ++buflen;
	icb_send(buf, buflen + 1);
}

void
icb_input(void)
{
	char *fields[ICB_MAXFIELDS];
	char *msg = netbuf;
	char type;
	size_t datalen, pos = 0;
	int i, nf = 1;

	datalen = (size_t)(unsigned char)msg[0] - 1;
	type = msg[1];
	msg += 2;
	for (i = 0; i < (int)datalen; i++)
		if (msg[i] == ICB_M_SEP)
			nf++;
	if (nf >= ICB_MAXFIELDS)
		return;
	bzero(fields, sizeof fields);
	for (i = 0; i < nf; i++)
		fields[i] = icb_parse(msg, datalen, &pos);
	if (loggedin == 0 && !(type == ICB_M_PROTO ||
	    type == ICB_M_LOGIN))
		errx(1, "wrong packet sequence");
	switch (type) {
	case ICB_M_PROTO:
		/* got proto packet - send login */
		icb_login();
		break;
	case ICB_M_LOGIN:
		/* initialize terminal after successfull login */
		loggedin++;
		icb_ttyinit();
		icb_display("Connection to \"%s\" at port \"%s\" "
		    "established\n", server, port); 
		break;
	case ICB_M_OPEN:
		icb_display("<%s> %s\n", fields[0], fields[1]);
		break;
	case ICB_M_PERSONAL:
		icb_display("*%s* %s\n", fields[0], fields[1]);
		break;
	case ICB_M_CMDOUT: {
		if (strcmp(fields[0], "wl") == 0) {
			if (nf < 8) {
				icb_display("[=Error=] Bad 'wl' "
				    " command output\n");
				return;
			}
			icb_display("  %c%s\t%ds\t%s@%s\t%s\n",
			    fields[1][0], fields[2], atoi(fields[3]),
			    fields[6], fields[7], fields[8]);
		} else if (strcmp(fields[0], "co") == 0) {
			icb_display("%s\n", fields[1]);
		} else if (strcmp(fields[0], "wh") == 0) {
			/* XXX: KM
			 * This is plain retarded so for now
			 * I will ignore it.
			 * icb_display("   Nick Idle\tAccount\n");
			 */
		}
	}
		break;
	case ICB_M_STATUS:
		icb_display("[=%s=] %s\n", fields[0], fields[1]);
		break;
	case ICB_M_ERROR:
		icb_display("[=Error=] %s\n", fields[0]);
		break;
	default:
		/* not valid */
		break;
	}
}


/*
 *  icb_parse: scans message for separators and returns pointer to the
 *             beginning of the field at position specified by 'pos'
 */
char *
icb_parse(char *msg, size_t len, size_t *pos)
{
	char *s = msg + *pos;
	char *t = s;
	size_t i;

	if (*pos > len)
		return (NULL);
	for (i = 0; i < len - *pos; i++) {
		if (t[i] == ICB_M_SEP) {
			t[i] = '\0';
			break;
		}
	}
	*pos += i + 1;
	return (s);
}

/* 
 *  icb_notify: send a notify package to the server to avoid a timeout
 */
void
icb_notify(int fd, short event, void *arg)
{
	struct timeval	t = { 20.0 , 0.0 };
	struct event *ev = (struct event *)arg;
	char buf[] = { 2, ICB_M_NOOP, '\0' };

	icb_send(buf, sizeof(buf));
	if (evtimer_add(ev, &t))
		err(1, "icb_notify");
}
