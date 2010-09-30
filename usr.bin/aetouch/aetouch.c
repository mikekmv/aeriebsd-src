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
#include <sys/queue.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <termios.h>
#include <util.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <signal.h>
#include <fcntl.h>

#include "aetouch.h"

void remove_client(int fd);
void master_signal(int sig);
int mkname(char**);
int mkunixsock(char *);
int add_client(int fd);
void pty_in(int, short, void*);
void accept_client(int, short, void*);
void client_command(int, short, void*);
void usage(void);
void aetexit(void);

#define	MAX_CON	140

char *aepath;		/* Socket Path */
int aemaster;		/* Master File Discriptor to pty */

SLIST_HEAD(listhaed, ae_client) ae_clients;
struct ae_client {
	SLIST_ENTRY(ae_client) entries;
	int fd;
};

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-dh] [-a file] [command ...]\n",
	    __progname);
	exit(1);
}

void
aetexit(void)
{
	unlink(aepath);
}

int
main(int argc, char **argv)
{
	struct event evs[3];
	struct termios term;
	pid_t child;
	int master_socket;
	int attached = 0;
	int client, nullfd, ch;

	while((ch = getopt(argc, argv, "dha:")) != -1) {
		switch(ch) {
		case 'a':
			attached++;
			aepath = optarg;
			break;
		case 'd':
			attached++;
			break;
		case 'h':
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (attached)
		return attach(aepath);
	else
		if (mkname(&aepath) < 0)
			err(1, "mkname");

	if (tcgetattr(0, &term) < 0)
		err(1, "tcgetattr");

	master_socket = mkunixsock(aepath);
	if (master_socket < 0)
		err(1, "mkunixsock: %s", aepath);

	child = fork();
	if (child > 0) {
		close(master_socket);
		return attach(aepath);
	} else if (child < 0) {
		warn("fork");
		unlink(aepath);
		exit(1);
	}

	if (atexit(&aetexit)) {
		warn("atexit");
		unlink(aepath);
		exit(1);
	}

	/* We wait for the first client before we start reading */
	if ((client = accept(master_socket, NULL, NULL)) < 0)
		err(1, "accept");

	SLIST_INIT(&ae_clients);
	add_client(client);

	child = forkpty(&aemaster, NULL, NULL, NULL);
	if (child == 0) {
		close(master_socket);
		if (execvp(argv[0], argv) < 0)
			err(1, "execvp: %s", argv[0]);
	} else if (child < 0)
		err(1, "forkpty");

	signal(SIGPIPE, SIG_IGN);
	signal(SIGXFSZ, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, master_signal);
	signal(SIGTERM, master_signal);
	signal(SIGCHLD, master_signal);

	/* Stop in- or output to console */
	nullfd = open("/dev/null", O_RDWR);
	dup2(nullfd, 0);
	dup2(nullfd, 1);
	dup2(nullfd, 2);
	if (nullfd > 2)
		close(nullfd);

	event_init();

	/* Add the pseudo tty to event list */
	event_set(&evs[0], aemaster, EV_READ|EV_PERSIST, pty_in, &evs[0]);
	if (event_add(&evs[0], NULL) < 0)
		err(1, "event_add0");

	/* Add master socket to event list */
	event_set(&evs[1], master_socket, EV_READ|EV_PERSIST, accept_client,
	    &evs[1]);
	if (event_add(&evs[1], NULL) < 0)
		err(1, "event_add1");

	/* Add the first connected client */
	event_set(&evs[2], client, EV_READ|EV_PERSIST, client_command, &evs[2]);
	if (event_add(&evs[2], NULL) < 0)
		err(1, "event_add2");

	event_dispatch();

	return 0;
}

int
mkname(char **path)
{
	extern char *__progname;
	int res;
	char *result;

	if (path == NULL)
		return -1;

	res = asprintf(&result, "/tmp/%s.%u.%u", __progname, getuid(),
	    getpid());
	if (res < 0)
		return -1;
	*path = result;
	return res;
}

int
mkunixsock(char *path)
{
	struct sockaddr_un sock;
	int s, err;

	if (!path)
		return -1;

	s = socket(PF_UNIX, SOCK_STREAM, 0);
	if (s < 0)
		return -1;

	strlcpy((&sock)->sun_path, path, sizeof((&sock)->sun_path));
	sock.sun_family = PF_UNIX;

	if (bind(s, (struct sockaddr *)&sock, sizeof(sock)) < 0) {
		err = errno;
		close(s);
		errno = err;
		return -1;
	}

	if (listen(s, MAX_CON) < 0) {
		err = errno;
		close(s);
		unlink(path);
		errno = err;
		return -1;
	}

	if (chmod(path, 0600) < 0) {
		err = errno;
		close(s);
		unlink(path);
		errno = err;
		return -1;
	}

	return s;
}

int
add_client(int fd)
{
	struct ae_client *a = NULL;

	if ((a = malloc(sizeof(struct ae_client))) == NULL)
		return -1;
	a->fd = fd;
	SLIST_INSERT_HEAD(&ae_clients, a, entries);

	return 0;
}

void
remove_client(int fd)
{
	struct ae_client *a;
	int found = 0;

	SLIST_FOREACH(a, &ae_clients, entries) {
		if(a->fd == fd) {
			found++;
			break;
		}
	}
	if (found) {
		close(a->fd);
		SLIST_REMOVE(&ae_clients, a, ae_client, entries);
	}
}

void
pty_in(int fd, short event, void *arg)
{
	struct ae_client *a;
	int written, len;
	char buf[BUFSIZE];

	len = read(fd, buf, sizeof(buf));
	if (len < 0)
		exit(1);

	SLIST_FOREACH(a, &ae_clients, entries) {
		written = 0;
		while(written < len) {
			int n = write(a->fd, buf + written, len -
			    written);
			if (n > 0) {
				written += n;
				continue;
			} else if (n < 0 && errno == EINTR)
				continue;
			break;
		}
	}
}

void
accept_client(int fd, short event, void *arg)
{
	struct event *ev;
	int client;

	if ((client = accept(fd, NULL, NULL)) < 0)
		return;
	if ((ev = calloc(1, sizeof(*ev))) == NULL)
		return;
	event_set(ev, client, EV_READ|EV_PERSIST, client_command, ev);
	if (event_add(ev, NULL) < 0) {
		close(client);
		return;
	}
	add_client(client);
}

void
client_command(int fd, short event, void *arg)
{
	int len;
	struct ae_msg am;

	len = read(fd, &am, sizeof(struct ae_msg));
	if (len < 0 && (errno == EAGAIN || errno == EINTR))
		return;

	if (len <= 0)
		remove_client(fd);

	switch(am.type) {
	case MSG_PUSH:
		write(aemaster, am.u.buf, am.len);
		break;
	case MSG_DTACH:
		remove_client(fd);
		break;
	case MSG_WINCH:
		ioctl(aemaster, TIOCSWINSZ, am.u.ws);
		break;
	default:
		return;
	}
}

void
master_signal(int sig)
{
	exit(1);
}
