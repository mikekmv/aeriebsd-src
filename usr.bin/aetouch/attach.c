/*
 * Copyright (c) 2009 Konrad Merz
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "aetouch.h"

#define	CONTROL(c)	((c)&037)

static int mksocket(char *);
static char *getdetached(void);
void master_in(int, short, void *);
void std_in(int, short, void *);
void restore(void);
void signal_handler(int sig);

static struct termios save_term;
static int m_socket;
static int ctrl_a = 0;

int
attach(char *path)
{
	struct event *ev;
	struct termios cur_term;
	struct ae_msg am;

	/* Get current terminal and save it in save_term */
	tcgetattr(0, &cur_term);
	save_term = cur_term;

	atexit(restore);

	cur_term.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|
	    ICRNL);
	cur_term.c_iflag &= ~(IXON|IXOFF);
	cur_term.c_oflag &= ~(OPOST);
	cur_term.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	cur_term.c_cflag &= ~(CSIZE|PARENB);
	cur_term.c_cflag |= CS8;
	cur_term.c_cc[VLNEXT] = _POSIX_VDISABLE;
	cur_term.c_cc[VMIN] = 1;
	cur_term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSADRAIN, &cur_term);

	signal(SIGPIPE, SIG_IGN);
	signal(SIGXFSZ, SIG_IGN);
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGWINCH, signal_handler);

	event_init();

	/* If no file was given get any form the user */
	if (path == NULL) {
		path = getdetached();
		/* No detached process found */
		if (path == NULL) {
			fprintf(stderr, "Nothing detached - %s"
			    " terminate now\n", __progname);
			return 1;
		}
	}

	m_socket = mksocket(path);
	if (m_socket < 0)
		err(1, "mksocket");

	if ((ev = calloc(1, sizeof(*ev))) == NULL)
		return 1;
	event_set(ev, 0, EV_READ|EV_PERSIST, std_in, ev);
	if (event_add(ev, NULL) < 0) {
		free(ev);
		return 1;
	}
	if ((ev = calloc(1, sizeof(*ev))) == NULL)
		return 1;
	event_set(ev, m_socket, EV_READ|EV_PERSIST, master_in, ev);
	if (event_add(ev, NULL) < 0) {
		free(ev);
		return 1;
	}

	/* Tell the master our win size */
	ioctl(0, TIOCGWINSZ, &am.u.ws);
	write(m_socket, &am, sizeof(am));

	event_dispatch();

	return 0;
}

void
master_in(int fd, short event __attribute__((__unused__)),
void *arg __attribute__((__unused__)))
{
	int len;
	char buf[BUFSIZE];

	len = read(fd, buf, sizeof(buf));
	if (len == 0) {
		printf("\r\n[EOF - aetouch terminating]\r\n");
		exit(0);
	} else if (len < 0) {
		printf("\r\n[EOS - read returned an error]\r\n");
		exit(0);
	}
	write(1, buf, len);
}

void
std_in(int fd, short event __attribute__((__unused__)),
void *arg __attribute__((__unused__)))
{
	struct ae_msg am;
	int i, end = 0;

	bzero(&am, sizeof(am));

	am.len = read(fd, am.u.buf, sizeof(am.u.buf));
	if (am.len < 0)
		exit(1);

	for (i = 0; i < am.len; i++) {
		switch(am.u.buf[i]) {
		case CONTROL('A'):
			/* A command was propably triggert */
			ctrl_a++;
			break;
		case 'd':
			if (ctrl_a) {
				am.type = MSG_DTACH;
				printf("\r\n[aetouch detached]\r\n");
				end++;
				break;
			}
			/* FALLTRHOUGH */
		default:
			if (ctrl_a > 0 && ctrl_a < 3)
				break;
			else
				ctrl_a = 0;
			am.type = MSG_PUSH;
			break;
		}
		if (end)
			exit(0);
	}

	write(m_socket, &am, sizeof(am));
}

static int
mksocket(char *path)
{
	struct sockaddr_un sock;
	int s;

	s = socket(PF_UNIX, SOCK_STREAM, 0);
	if (s < 0)
		return -1;

	sock.sun_family = PF_UNIX;
	strlcpy((&sock)->sun_path, path, sizeof((&sock)->sun_path));
	if (connect(s, (struct sockaddr *)&sock, sizeof(sock)) < 0) {
		close(s);
		return -1;
	}
	return (s);
}

void
restore(void)
{
	/* Restor TERM */
 	tcsetattr(0, TCSADRAIN, &save_term);
}

void
signal_handler(int sig)
{
	struct ae_msg am;

	switch(sig) {
	case SIGHUP:
	case SIGINT:
		printf("\r\n[aetouch detached]\r\n");
		exit(1);
		/* NOT REACHED */
	case SIGWINCH:
		signal(SIGWINCH, signal_handler);
		am.type = MSG_WINCH;
		ioctl(0, TIOCGWINSZ, &am.u.ws);
		write(m_socket, &am, sizeof(am));
		break;
	default:
		break;
	}
}

static char *
getdetached(void)
{
	DIR *dirp;
	struct dirent *de;
	int found = 0;
	int len;
	char *fname;

	len = asprintf(&fname, "%s.%u.", __progname, getuid());
	if (len < 0)
		return NULL;

	dirp = opendir("/tmp");
	while ((de = readdir(dirp)) != NULL) {
		if (de->d_namlen < len)
			continue;
		if (strncmp(de->d_name, fname, len) == 0) {
			found++;
			break;
		}
	}
	free(fname);
	fname = NULL;
	if (found)
		if (asprintf(&fname, "/tmp/%s", de->d_name) < 0)
			return NULL;
	closedir(dirp);
	return fname;
}
