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

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <termios.h>
#include <util.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <signal.h>

#include <paths.h>

#include "aetouch.h"

#define	CONTROL(c)	((c)&037)

int mksocket(char *);
char *getdetached(void);
void master_in(int, short, void *);
void std_in(int, short, void *);
void restore(void);
void signal_handler(int sig);

struct termios save_term;
int m_socket;
int ctrl_a = 0;

int
attach(char *path)
{
	struct event evs[2];
	struct termios cur_term;
	struct ae_msg am;

	/* If no file was given get any form the user */
	if (path == NULL) {
		path = getdetached();
		/* No detached process found */
		if (path == NULL) {
			warnx("Nothing detached");
			return 1;
		}
	}

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

	m_socket = mksocket(path);
	if (m_socket < 0)
		err(1, "mksocket: %s", path);

	event_set(&evs[0], 0, EV_READ|EV_PERSIST, std_in, &evs[0]);
	if (event_add(&evs[0], NULL) < 0)
		err(1, "event_add0");

	event_set(&evs[1], m_socket, EV_READ|EV_PERSIST, master_in, &evs[1]);
	if (event_add(&evs[1], NULL) < 0)
		err(1, "event_add1");

	/* Tell the master our win size */
	ioctl(0, TIOCGWINSZ, &am.u.ws);
	write(m_socket, &am, sizeof(am));

	event_dispatch();

	return 0;
}

void
master_in(int fd, short event, void *arg)
{
	int len;
	char buf[BUFSIZE];

	len = read(fd, buf, sizeof(buf));
	if (len == 0) {
		printf("\r\n[EOF - terminating]\r\n");
		exit(0);
	} else if (len < 0) {
		printf("\r\n[EOS - read returned an error]\r\n");
		exit(0);
	}
	write(1, buf, len);
}

void
std_in(int fd, short event, void *arg)
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
				printf("\r\n[detached]\r\n");
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

int
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
	/* Restore TERM */
	tcsetattr(0, TCSADRAIN, &save_term);
}

void
signal_handler(int sig)
{
	struct ae_msg am;

	switch(sig) {
	case SIGHUP:
	case SIGINT:
		printf("\r\n[detached]\r\n");
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

char *
getdetached(void)
{
	extern char *__progname;
	DIR *dirp;
	struct dirent *de;
	int len, found = 0;
	char *fname;

	len = asprintf(&fname, "%s.%u.", __progname, getuid());
	if (len < 0)
		return NULL;

	dirp = opendir(_PATH_TMP);
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
		if (asprintf(&fname, "%s%s", _PATH_TMP, de->d_name) < 0)
			return NULL;
	closedir(dirp);
	return fname;
}
