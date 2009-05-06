/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1988 Mark Nudleman.\n"
"@(#) Copyright (c) 1988, 1993"
"	Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)main.c	8.1 (Berkeley) 6/7/93";
static char sccsid[] = "@(#)option.c	8.1 (Berkeley) 6/6/93";
static char sccsid[] = "@(#)signal.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] =
    "$ABSD$";
#endif
#endif /* not lint */

/*
 * Entry point, initialization, miscellaneous routines.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <curses.h>
#include <term.h>
#include <errno.h>

#include "less.h"

int	ispipe, new_file, is_tty;
char	*current_file, *previous_file, *current_name, *next_name;
off_t	prev_pos;
int	any_display, nscroll;
int	ac, curr_ac;
char	**av;
int	quitting;
int	top_scroll;		/* Repaint screen from top */
int	bs_mode;		/* How to process backspaces */
int	caseless;		/* Do "caseless" searches */
int	cbufs = 10;		/* Current number of buffers */
int	linenums = 1;		/* Use line numbers */
int	quit_at_eof;
int	squeeze;		/* Squeeze multiple blank lines into one */
int	tabstop = 8;		/* Tab settings */
int	tagoption;
int	sc_window_set;

char *tags = "tags";
char *firstsearch;

/*
 * "sigs" contains bits indicating signals which need to be processed.
 */
int sigs;

int option(int, char **, char **, char **);
char *xstrdup(const char *);
void purgeandquit(int);
void cat_file(void);
void stop(int sig);

/*
 * Edit a new file.
 * Filename "-" means standard input.
 * No filename means the "current" file, from the command line.
 */
int
edit(char *filename)
{
	static int didpipe;
	char message[100], *m;
	off_t initial_pos;
	int f;

	initial_pos = NULL_POSITION;
	if (filename == NULL || *filename == '\0') {
		if (curr_ac >= ac) {
			error("No current file");
			return(0);
		}
		filename = xstrdup(av[curr_ac]);
	} else if (strcmp(filename, "#") == 0) {
		if (*previous_file == '\0') {
			error("no previous file");
			return(0);
		}
		filename = xstrdup(previous_file);
		initial_pos = prev_pos;
	} else
		filename = xstrdup(filename);

	/* use standard input. */
	if (!strcmp(filename, "-")) {
		if (didpipe) {
			error("Can view standard input only once");
			return(0);
		}
		f = 0;
	} else if ((m = bad_file(filename, message, sizeof(message))) != NULL) {
		error(m);
		free(filename);
		return(0);
	} else if ((f = open(filename, O_RDONLY, 0)) < 0) {
		snprintf(message, sizeof(message),
		    "%s: %s", filename, strerror(errno));
		error(message);
		free(filename);
		return(0);
	}

	if (isatty(f)) {
		/*
		 * Not really necessary to call this an error,
		 * but if the control terminal (for commands)
		 * and the input file (for data) are the same,
		 * we get weird results at best.
		 */
		error("Can't take input from a terminal");
		if (f > 0)
			(void)close(f);
		(void)free(filename);
		return(0);
	}

	/*
	 * We are now committed to using the new file.
	 * Close the current input file and set up to use the new one.
	 */
	if (file > 0)
		(void)close(file);
	new_file = 1;
	if (previous_file != NULL)
		free(previous_file);
	previous_file = current_file;
	current_file = filename;
	pos_clear();
	prev_pos = position(TOP);
	ispipe = (f == 0);
	if (ispipe) {
		didpipe = 1;
		current_name = "stdin";
	} else
		current_name = basename(filename);
	if (curr_ac >= ac)
		next_name = NULL;
	else
		next_name = av[curr_ac + 1];
	file = f;
	ch_init(cbufs, 0);
	init_mark();

	if (is_tty) {
		int no_display = !any_display;
		any_display = 1;
		if (no_display && errmsgs > 0) {
			/*
			 * We displayed some messages on error output
			 * (file descriptor 2; see error() function).
			 * Before erasing the screen contents,
			 * display the file name and wait for a keystroke.
			 */
			error(filename);
		}
		/*
		 * Indicate there is nothing displayed yet.
		 */
		if (initial_pos != NULL_POSITION)
			jump_loc(initial_pos);
		clr_linenum();
	}
	return(1);
}

/*
 * Edit the next file in the command line list.
 */
void
next_file(int n)
{
	if (curr_ac + n >= ac) {
		if (quit_at_eof || position(TOP) == NULL_POSITION)
			quit();
		error("No (N-th) next file");
	} else
		(void)edit(av[curr_ac += n]);
}

/*
 * Edit the previous file in the command line list.
 */
void
prev_file(int n)
{
	if (curr_ac - n < 0)
		error("No (N-th) previous file");
	else
		(void)edit(av[curr_ac -= n]);
}

/*
 * copy a file directly to standard output; used if stdout is not a tty.
 * the only processing is to squeeze multiple blank input lines.
 */
void
cat_file(void)
{
	int c, empty;

	if (squeeze) {
		empty = 0;
		while ((c = ch_forw_get()) != EOI)
			if (c != '\n') {
				putchar(c);
				empty = 0;
			} else if (empty < 2) {
				putchar(c);
				++empty;
			}
	}
	else while ((c = ch_forw_get()) != EOI)
		putchar(c);
	fflush(stdout);
}

int
main(int argc, char *argv[])
{
	int envargc, argcnt;
	char *envargv[2], *tagfile, *patt;

	/*
	 * Process command line arguments and MORE environment arguments.
	 * Command line arguments override environment arguments.
	 */
	if ((envargv[1] = getenv("MORE"))) {
		envargc = 2;
		envargv[0] = "more";
		envargv[2] = NULL;
		(void)option(envargc, envargv, &tagfile, &patt);
	}
	argcnt = option(argc, argv, &tagfile, &patt);
	argv += argcnt;
	argc -= argcnt;

	/*
	 * Set up list of files to be examined.
	 */
	ac = argc;
	av = argv;
	curr_ac = 0;

	/*
	 * Set up terminal, etc.
	 */
	is_tty = isatty(1);
	if (!is_tty) {
		/*
		 * Output is not a tty.
		 * Just copy the input file(s) to output.
		 */
		if (ac < 1) {
			(void)edit("-");
			cat_file();
		} else {
			do {
				(void)edit((char *)NULL);
				if (file >= 0)
					cat_file();
			} while (++curr_ac < ac);
		}
		exit(0);
	}

	raw_mode(1);
	get_term();
	init();
	init_signals(1);

	/* select the first file to examine. */
	if (tagoption) {
		/*
		 * A -t option was given; edit the file selected by the
		 * "tags" search, and search for the proper line in the file.
		 */
		if (!tagfile || !edit(tagfile) || tagsearch(patt))
			quit();
	}
	else if (ac < 1)
		(void)edit("-");	/* Standard input */
	else {
		/*
		 * Try all the files named as command arguments.
		 * We are simply looking for one which can be
		 * opened without error.
		 */
		do {
			(void)edit((char *)NULL);
		} while (file < 0 && ++curr_ac < ac);
	}

	if (file >= 0)
		commands();
	quit();
	/*NOTREACHED*/
	return (1);
}

/*
 * Copy a string to a "safe" place
 * (that is, to a buffer allocated by malloc).
 */
char *
xstrdup(const char *s)
{
	char *p;

	p = strdup(s);
	if (p == NULL) {
		error("cannot allocate memory");
		quit();
	}
	return p;
}

/*
 * Exit the program.
 */
void
quit(void)
{
	/*
	 * Put cursor at bottom left corner, clear the line,
	 * reset the terminal modes, and exit.
	 */
	quitting = 1;
	lower_left();
	clear_eol();
	deinit();
	fflush(stdout);
	raw_mode(0);
	exit(0);
}

int
option(int argc, char **argv, char **tagfile, char **patt)
{
	extern char *__progname;
	extern char *optarg;
	extern int optind;
	int ch;
	char *p, **a;

	/* backward compatible processing for "+/search" */
	for (a = argv; *a; ++a)
		if ((*a)[0] == '+' && (*a)[1] == '/')
			(*a)[0] = '-';

	optind = 1;		/* called twice, re-init getopt. */
	while ((ch = getopt(argc, argv, "0123456789/:ceinst:ux:f")) != EOF)
		switch((char)ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			/*
			 * kludge: more was originally designed to take
			 * a number after a dash.
			 */
			if (!sc_window_set) {
				p = argv[optind - 1];
				if (p[0] == '-' && p[1] == ch && !p[2])
					sc_height = atoi(++p);
				else
					sc_height = atoi(argv[optind] + 1);
				sc_window_set = 1;
			}
			break;
		case '/':
			firstsearch = optarg;
			break;
		case 'c':
			top_scroll = 1;
			break;
		case 'e':
			quit_at_eof = 1;
			break;
		case 'i':
			caseless = 1;
			break;
		case 'n':
			linenums = 0;
			break;
		case 's':
			squeeze = 1;
			break;
		case 't':
			tagoption = 1;
			findtag(optarg, tagfile, patt);
			break;
		case 'u':
			bs_mode = 1;
			break;
		case 'x':
			tabstop = atoi(optarg);
			if (tabstop <= 0)
				tabstop = 8;
			break;
		case 'f':	/* ignore -f, compatability with old more */
			break;
		case '?':
		default:
			fprintf(stderr,
			    "usage: %s [-ceinus] [-t tag] [-x tabs] [-/ pattern] [-#] [file ...]\n", __progname);
			exit(1);
		}
	return(optind);
}

/*
 * Routines dealing with signals.
 *
 * A signal usually merely causes a bit to be set in the "signals" word.
 * At some convenient time, the mainline code checks to see if any
 * signals need processing by calling psignal().
 * If we happen to be reading from a file [in iread()] at the time
 * the signal is received, we call intread to interrupt the iread.
 */

#ifdef SIGTSTP
#define	S_STOP		02
#endif
#if defined(SIGWINCH) || defined(SIGWIND)
#define S_WINCH		04
#endif

#ifdef SIGTSTP
/*
 * "Stop" (^Z) signal handler.
 */
void
stop(int sig)
{
	(void)signal(SIGTSTP, stop);
	sigs |= S_STOP;
	if (reading)
		intread();
}
#endif

/*
 * "Window" change handler
 */
void
windoch(int sig)
{
	(void)signal(SIGWINCH, windoch);
	sigs |= S_WINCH;
	if (reading)
		intread();
}

void
purgeandquit(int sig)
{
	fpurge(stdout);	/* purge buffered output */
	quit();
}

/*
 * Set up the signal handlers.
 */
void
init_signals(int on)
{
	if (on) {
		/*
		 * Set signal handlers.
		 */
		(void)signal(SIGINT, purgeandquit);
#ifdef SIGTSTP
		(void)signal(SIGTSTP, stop);
#endif
#ifdef SIGWINCH
		(void)signal(SIGWINCH, windoch);
#else
#ifdef SIGWIND
		(void)signal(SIGWIND, windoch);
#endif
#endif
	} else {
		/*
		 * Restore signals to defaults.
		 */
		(void)signal(SIGINT, SIG_DFL);
#ifdef SIGTSTP
		(void)signal(SIGTSTP, SIG_DFL);
#endif
#ifdef SIGWINCH
		(void)signal(SIGWINCH, SIG_IGN);
#endif
#ifdef SIGWIND
		(void)signal(SIGWIND, SIG_IGN);
#endif
	}
}

/*
 * Process any signals we have received.
 * A received signal cause a bit to be set in "sigs".
 */
void
psignals(void)
{
	int tsignals;

	if ((tsignals = sigs) == 0)
		return;
	sigs = 0;

#ifdef S_WINCH
	if (tsignals & S_WINCH) {
		int old_width, old_height;
		/*
		 * Re-execute get_term() to read the new window size.
		 */
		old_width = sc_width;
		old_height = sc_height;
		get_term();
		if (sc_width != old_width || sc_height != old_height) {
			nscroll = (sc_height + 1) / 2;
			screen_trashed = 1;
		}
	}
#endif
#ifdef SIGTSTP
	if (tsignals & S_STOP) {
		/*
		 * Clean up the terminal.
		 */
#ifdef SIGTTOU
		(void)signal(SIGTTOU, SIG_IGN);
#endif
		lower_left();
		clear_eol();
		deinit();
		fflush(stdout);
		raw_mode(0);
#ifdef SIGTTOU
		(void)signal(SIGTTOU, SIG_DFL);
#endif
		(void)signal(SIGTSTP, SIG_DFL);
		(void)kill(getpid(), SIGTSTP);
		/*
		 * ... Bye bye. ...
		 * Hopefully we'll be back later and resume here...
		 * Reset the terminal and arrange to repaint the
		 * screen when we get back to the main command loop.
		 */
		(void)signal(SIGTSTP, stop);
		raw_mode(1);
		init();
		screen_trashed = 1;
	}
#endif
}
