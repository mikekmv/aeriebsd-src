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
#if 0
static char sccsid[] = "@(#)command.c	8.1 (Berkeley) 6/6/93";
static char sccsid[] = "@(#)help.c	8.1 (Berkeley) 6/6/93";
static char sccsid[] = "@(#)os.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] =
    "$ABSD$";
#endif
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <term.h>
#include <ctype.h>
#include <errno.h>
#include <paths.h>

#include "less.h"

#define	NO_MCA		0
#define	MCA_DONE	1
#define	MCA_MORE	2

static char cmdbuf[120];	/* Buffer for holding a multi-char command */
static char *cp;		/* Pointer into cmdbuf */
static int cmd_col;		/* Current column of the multi-char command */
static int longprompt;		/* if stat command instead of prompt */
static int mca;			/* The multicharacter command (action) */
static int last_mca;		/* The previous mca */
static int number;		/* The number typed by the user */
static int wsearch;		/* Search for matches (1) or non-matches (0) */
int reading;
static jmp_buf read_label;

#define	CMD_RESET	cp = cmdbuf	/* reset command buffer to empty */
#define	CMD_EXEC	lower_left(); fflush(stdout)

void help(void);
char *glob(char *);
void lsystem(const char *);
void showlist(void);
void editfile(void);
void exec_mca(void);
int mca_char(int);
int getcc(void);
int prompt(void);
int cmd_erase(void);
int cmd_char(int);

/* backspace in command buffer. */
int
cmd_erase(void)
{
	/*
	 * backspace past beginning of the string: this usually means
	 * abort the command.
	 */
	if (cp == cmdbuf)
		return(1);

	/* erase an extra character, for the carat. */
	if (CONTROL_CHAR(*--cp)) {
		backspace();
		--cmd_col;
	}

	backspace();
	--cmd_col;
	return(0);
}

/* set up the display to start a new multi-character command. */
void
start_mca(int action, const char *str)
{
	lower_left();
	clear_eol();
	fputs(str, stdout);
	cmd_col = strlen(str);
	mca = action;
}

/*
 * process a single character of a multi-character command, such as
 * a number, or the pattern of a search command.
 */
int
cmd_char(int c)
{
	if (c == erase_char)
		return(cmd_erase());
	/* in this order, in case werase == erase_char */
	if (c == werase_char) {
		if (cp > cmdbuf) {
			while (isspace(cp[-1]) && !cmd_erase());
			while (!isspace(cp[-1]) && !cmd_erase());
			while (isspace(cp[-1]) && !cmd_erase());
		}
		return(cp == cmdbuf);
	}
	if (c == kill_char) {
		while (!cmd_erase());
		return(1);
	}
	/*
	 * No room in the command buffer, or no room on the screen;
	 * {{ Could get fancy here; maybe shift the displayed line
	 * and make room for more chars, like ksh. }}
	 */
	if (cp >= &cmdbuf[sizeof(cmdbuf)-1] || cmd_col >= sc_width-3)
		putchar('\7');
	else {
		*cp++ = c;
		if (CONTROL_CHAR(c)) {
			putchar('^');
			cmd_col++;
			c = CARAT_CHAR(c);
		}
		putchar(c);
		cmd_col++;
	}
	return(0);
}

int
prompt(void)
{
	off_t len, pos;

	/*
	 * if nothing is displayed yet, display starting from line 1;
	 * if search string provided, go there instead.
	 */
	if (position(TOP) == NULL_POSITION) {
		if (forw_line((off_t)0) == NULL_POSITION)
			return(0);
		if (!firstsearch || !search(1, firstsearch, 1, 1))
			jump_back(1);
	} else if (screen_trashed)
		repaint();

	/* if no -e flag and we've hit EOF on the last file, quit. */
	if ((!quit_at_eof || short_file) && hit_eof && curr_ac + 1 >= ac)
		quit();

	/* select the proper prompt and display it. */
	lower_left();
	clear_eol();
	if (longprompt) {
		so_enter();
		printf("%s:", current_name);
		if (!ispipe)
			printf(" file %d/%d", curr_ac + 1, ac);
		if (linenums)
			printf(" line %d", currline(BOTTOM));
		if ((pos = position(BOTTOM)) != NULL_POSITION) {
			printf(" byte %qd", pos);
			if (!ispipe && (len = ch_length()))
				printf("/%qd pct %qd%%",
				    len, ((100 * pos) / len));
		}
		so_exit();
		longprompt = 0;
	} else {
		so_enter();
		printf("%s", current_name);
		if (hit_eof)
			if (next_name)
				printf(": END (next file: %s)", next_name);
			else
				printf(": END");
		else if (!ispipe &&
		    (pos = position(BOTTOM)) != NULL_POSITION &&
		    (len = ch_length()))
			printf(" (%qd%%)", ((100 * pos) / len));
		so_exit();
	}
	return(1);
}

/* get command character. */
int
getcc(void)
{
	int ch;

	/* left over from error() routine. */
	if (cmdstack) {
		ch = cmdstack;
		cmdstack = NULL;
		return(ch);
	}
	if (cp > cmdbuf && position(TOP) == NULL_POSITION) {
		/*
		 * Command is incomplete, so try to complete it.
		 * There are only two cases:
		 * 1. We have "/string" but no newline.  Add the \n.
		 * 2. We have a number but no command.  Treat as #g.
		 * (This is all pretty hokey.)
		 */
		if (mca != A_DIGIT)
			/* Not a number; must be search string */
			return('\n');
		else
			/* A number; append a 'g' */
			return('g');
	}
	return(getchr());
}

/* execute a multicharacter command. */
void
exec_mca(void)
{
	char *p, *fn, *patt;

	*cp = '\0';
	CMD_EXEC;
	switch (mca) {
	case A_F_SEARCH:
		(void)search(1, cmdbuf, number, wsearch);
		break;
	case A_B_SEARCH:
		(void)search(0, cmdbuf, number, wsearch);
		break;
	case A_EXAMINE:
		for (p = cmdbuf; isspace(*p); ++p)
			;
		edit(glob(p));
		break;
	case A_TAGFILE:
		for (p = cmdbuf; isspace(*p); ++p);
		findtag(p, &fn, &patt);
		if (fn == NULL)
			break;
		if (edit(fn))
			(void)tagsearch(patt);
		break;
	}
}

/* add a character to a multi-character command. */
int
mca_char(int c)
{
	switch (mca) {
	case 0:			/* not in a multicharacter command. */
	case A_PREFIX:		/* in the prefix of a command. */
		return(NO_MCA);
	case A_DIGIT:
		/*
		 * Entering digits of a number.
		 * Terminated by a non-digit.
		 */
		if ((!isascii(c) || !isdigit(c)) &&
		    c != erase_char && c != kill_char && c != werase_char) {
			/*
			 * Not part of the number.
			 * Treat as a normal command character.
			 */
			*cp = '\0';
			number = atoi(cmdbuf);
			CMD_RESET;
			mca = 0;
			return(NO_MCA);
		}
		break;
	}

	/*
	 * Any other multicharacter command
	 * is terminated by a newline.
	 */
	if (c == '\n' || c == '\r') {
		exec_mca();
		return(MCA_DONE);
	}

	/* append the char to the command buffer. */
	if (cmd_char(c))
		return(MCA_DONE);

	return(MCA_MORE);
}

/*
 * Main command processor.
 * Accept and execute commands until a quit command, then return.
 */
void
commands(void)
{
	int c, action;

	last_mca = 0;
	nscroll = (sc_height + 1) / 2;

	for (;;) {
		mca = 0;
		number = 0;

		/*
		 * See if any signals need processing.
		 */
		if (sigs) {
			psignals();
			if (quitting)
				quit();
		}
		/*
		 * Display prompt and accept a character.
		 */
		CMD_RESET;
		if (!prompt()) {
			next_file(1);
			continue;
		}
		noprefix();
		c = getcc();

again:		if (sigs)
			continue;

		/*
		 * If we are in a multicharacter command, call mca_char.
		 * Otherwise we call cmd_decode to determine the
		 * action to be performed.
		 */
		if (mca)
			switch (mca_char(c)) {
			case MCA_MORE:
				/*
				 * Need another character.
				 */
				c = getcc();
				goto again;
			case MCA_DONE:
				/*
				 * Command has been handled by mca_char.
				 * Start clean with a prompt.
				 */
				continue;
			case NO_MCA:
				/*
				 * Not a multi-char command
				 * (at least, not anymore).
				 */
				break;
			}

		/* decode the command character and decide what to do. */
		switch (action = cmd_decode(c)) {
		case A_DIGIT:		/* first digit of a number */
			start_mca(A_DIGIT, ":");
			goto again;
		case A_F_SCREEN:	/* forward one screen */
			CMD_EXEC;
			if (number <= 0 && (number = sc_window) <= 0)
				number = sc_height - 1;
			forward(number, 1);
			break;
		case A_B_SCREEN:	/* backward one screen */
			CMD_EXEC;
			if (number <= 0 && (number = sc_window) <= 0)
				number = sc_height - 1;
			backward(number, 1);
			break;
		case A_F_LINE:		/* forward N (default 1) line */
			CMD_EXEC;
			forward(number <= 0 ? 1 : number, 0);
			break;
		case A_B_LINE:		/* backward N (default 1) line */
			CMD_EXEC;
			backward(number <= 0 ? 1 : number, 0);
			break;
		case A_F_SCROLL:	/* forward N lines */
			CMD_EXEC;
			if (number > 0)
				nscroll = number;
			forward(nscroll, 0);
			break;
		case A_B_SCROLL:	/* backward N lines */
			CMD_EXEC;
			if (number > 0)
				nscroll = number;
			backward(nscroll, 0);
			break;
		case A_FREPAINT:	/* flush buffers and repaint */
			if (!ispipe) {
				ch_init(0, 0);
				clr_linenum();
			}
			/* FALLTHROUGH */
		case A_REPAINT:		/* repaint the screen */
			CMD_EXEC;
			repaint();
			break;
		case A_GOLINE:		/* go to line N, default 1 */
			CMD_EXEC;
			if (number <= 0)
				number = 1;
			jump_back(number);
			break;
		case A_PERCENT:		/* go to percent of file */
			CMD_EXEC;
			if (number < 0)
				number = 0;
			else if (number > 100)
				number = 100;
			jump_percent(number);
			break;
		case A_GOEND:		/* go to line N, default end */
			CMD_EXEC;
			if (number <= 0)
				jump_forw();
			else
				jump_back(number);
			break;
		case A_STAT:		/* print file name, etc. */
			longprompt = 1;
			continue;
		case A_QUIT:		/* exit */
			quit();
		case A_F_SEARCH:	/* search for a pattern */
		case A_B_SEARCH:
			if (number <= 0)
				number = 1;
			start_mca(action, (action==A_F_SEARCH) ? "/" : "?");
			last_mca = mca;
			wsearch = 1;
			c = getcc();
			if (c == '!') {
				/*
				 * Invert the sense of the search; set wsearch
				 * to 0 and get a new character for the start
				 * of the pattern.
				 */
				start_mca(action, 
				    (action == A_F_SEARCH) ? "!/" : "!?");
				wsearch = 0;
				c = getcc();
			}
			goto again;
		case A_AGAIN_SEARCH:		/* repeat previous search */
			if (number <= 0)
				number = 1;
			if (wsearch)
				start_mca(last_mca, 
				    (last_mca == A_F_SEARCH) ? "/" : "?");
			else
				start_mca(last_mca, 
				    (last_mca == A_F_SEARCH) ? "!/" : "!?");
			CMD_EXEC;
			(void)search(mca == A_F_SEARCH, (char *)NULL,
			    number, wsearch);
			break;
		case A_HELP:			/* help */
			lower_left();
			clear_eol();
			fputs("help", stdout);
			CMD_EXEC;
			help();
			break;
		case A_TAGFILE:			/* tag a new file */
			CMD_RESET;
			start_mca(A_TAGFILE, "Tag: ");
			c = getcc();
			goto again;
		case A_FILE_LIST:		/* show list of file names */
			CMD_EXEC;
			showlist();
			repaint();
			break;
		case A_EXAMINE:			/* edit a new file */
			CMD_RESET;
			start_mca(A_EXAMINE, "Examine: ");
			c = getcc();
			goto again;
		case A_VISUAL:			/* invoke the editor */
			if (ispipe) {
				error("Cannot edit standard input");
				break;
			}
			CMD_EXEC;
			editfile();
			ch_init(0, 0);
			clr_linenum();
			break;
		case A_NEXT_FILE:		/* examine next file */
			if (number <= 0)
				number = 1;
			next_file(number);
			break;
		case A_PREV_FILE:		/* examine previous file */
			if (number <= 0)
				number = 1;
			prev_file(number);
			break;
		case A_SETMARK:			/* set a mark */
			lower_left();
			clear_eol();
			start_mca(A_SETMARK, "mark: ");
			c = getcc();
			if (c == erase_char || c == kill_char)
				break;
			setmark(c);
			break;
		case A_GOMARK:			/* go to mark */
			lower_left();
			clear_eol();
			start_mca(A_GOMARK, "goto mark: ");
			c = getcc();
			if (c == erase_char || c == kill_char)
				break;
			gomark(c);
			break;
		case A_PREFIX:
			/*
			 * The command is incomplete (more chars are needed).
			 * Display the current char so the user knows what's
			 * going on and get another character.
			 */
			if (mca != A_PREFIX)
				start_mca(A_PREFIX, "");
			if (CONTROL_CHAR(c)) {
				putchar('^');
				c = CARAT_CHAR(c);
			}
			putchar(c);
			c = getcc();
			goto again;
		default:
			putchar('\7');
			break;
		}
	}
}

void
editfile(void)
{
	static int dolinenumber;
	static char *editor;
	int c;
	char buf[MAXPATHLEN * 2 + 20];

	if (editor == NULL) {
		editor = getenv("EDITOR");
		/* pass the line number to vi */
		if (editor == NULL || *editor == '\0') {
			editor = _PATH_VI;
			dolinenumber = 1;
		}
		else
			dolinenumber = 0;
	}
	if (dolinenumber && (c = currline(MIDDLE)))
		snprintf(buf, sizeof(buf),
		    "%s +%d %s", editor, c, current_file);
	else
		snprintf(buf, sizeof(buf), "%s %s", editor, current_file);
	lsystem(buf);
}

void
showlist(void)
{
	int indx, width, len;
	char *p;

	if (ac <= 0) {
		error("No files provided as arguments.");
		return;
	}
	for (width = indx = 0; indx < ac;) {
		p = strcmp(av[indx], "-") ? av[indx] : "stdin";
		len = strlen(p) + 1;
		if (curr_ac == indx)
			len += 2;
		if (width + len + 1 >= sc_width) {
			if (!width) {
				if (curr_ac == indx)
					putchar('[');
				fputs(p, stdout);
				if (curr_ac == indx)
					putchar(']');
				++indx;
			}
			width = 0;
			putchar('\n');
			continue;
		}
		if (width)
			putchar(' ');
		if (curr_ac == indx)
			putchar('[');
		fputs(p, stdout);
		if (curr_ac == indx)
			putchar(']');
		width += len;
		++indx;
	}
	putchar('\n');
	error(NULL);
}

void
help(void)
{
	char cmd[MAXPATHLEN + 20];

	snprintf(cmd, sizeof(cmd), "-more %s", _PATH_HELPFILE);
	lsystem(cmd);
}

/*
 * Pass the specified command to a shell to be executed.
 * Like plain "system()", but handles resetting terminal modes, etc.
 */
void
lsystem(const char *cmd)
{
	int inp;
	char sysbuf[256];
	char *shell;

	/*
	 * Print the command which is to be executed,
	 * unless the command starts with a "-".
	 */
	if (cmd[0] == '-')
		cmd++;
	else {
		lower_left();
		clear_eol();
		printf("!%s\n", cmd);
	}

	/*
	 * De-initialize the terminal and take out of raw mode.
	 */
	deinit();
	fflush(stdout);
	raw_mode(0);

	/*
	 * Restore signals to their defaults.
	 */
	init_signals(0);

	/*
	 * Force standard input to be the terminal, "/dev/tty",
	 * even if less's standard input is coming from a pipe.
	 */
	inp = dup(0);
	(void)close(0);
	if (open(_PATH_TTY, O_RDONLY, 0) < 0)
		(void)dup(inp);

	/*
	 * Pass the command to the system to be executed.
	 * If we have a SHELL environment variable, use
	 * <$SHELL -c "command"> instead of just <command>.
	 * If the command is empty, just invoke a shell.
	 */
	if ((shell = getenv("SHELL")) != NULL && *shell != '\0') {
		if (*cmd == '\0')
			cmd = shell;
		else {
			snprintf(sysbuf, sizeof(sysbuf),
			    "%s -c \"%s\"", shell, cmd);
			cmd = sysbuf;
		}
	}
	if (*cmd == '\0')
		cmd = "sh";

	(void)system(cmd);

	/*
	 * Restore standard input, reset signals, raw mode, etc.
	 */
	(void)close(0);
	(void)dup(inp);
	(void)close(inp);

	init_signals(1);
	raw_mode(1);
	init();
	screen_trashed = 1;
	/*
	 * Since we were ignoring window change signals while we executed
	 * the system command, we must assume the window changed.
	 */
	windoch(0);
}

/*
 * Like read() system call, but is deliberately interruptable.
 * A call to intread() from a signal handler will interrupt
 * any pending iread().
 */
int
iread(int fd, char *buf, int len)
{
	int n;

	if (setjmp(read_label))
		/*
		 * We jumped here from intread.
		 */
		return (READ_INTR);

	fflush(stdout);
	reading = 1;
	n = read(fd, buf, len);
	reading = 0;
	if (n < 0)
		return (-1);
	return (n);
}

void
intread(void)
{
	(void)sigsetmask(0L);
	longjmp(read_label, 1);
}

/*
 * Expand a filename, substituting any environment variables, etc.
 * The implementation of this is necessarily very operating system
 * dependent.  This implementation is unabashedly only for Unix systems.
 *
 * XXX adopt to a glob(3)
 */
char *
glob(char *filename)
{
	static char buffer[MAXPATHLEN];
	FILE *f;
	char *p, *cmd;
	int ch;
	size_t len;

	if (filename[0] == '#')
		return (filename);

	/*
	 * We get the shell to expand the filename for us by passing
	 * an "echo" command to the shell and reading its output.
	 */
	p = getenv("SHELL");
	if (p == NULL || *p == '\0') {
		/*
		 * Read the output of <echo filename>.
		 */
		cmd = malloc(len = strlen(filename) + 8);
		if (cmd == NULL)
			return (filename);
		snprintf(cmd, len, "echo \"%s\"", filename);
	} else {
		/*
		 * Read the output of <$SHELL -c "echo filename">.
		 */
		cmd = malloc(len = strlen(p) + 12);
		if (cmd == NULL)
			return (filename);
		snprintf(cmd, len, "%s -c \"echo %s\"", p, filename);
	}

	if ((f = popen(cmd, "r")) == NULL)
		return (filename);
	free(cmd);

	for (p = buffer;  p < &buffer[sizeof(buffer)-1];  p++) {
		if ((ch = getc(f)) == '\n' || ch == EOF)
			break;
		*p = ch;
	}
	*p = '\0';
	(void)pclose(f);
	return(buffer);
}

char *
bad_file(const char *filename, char *message, size_t len)
{
	struct stat sb;

	if (stat(filename, &sb) < 0) {
		snprintf(message, len, "%s: %s", filename, strerror(errno));
		return (message);
	}

	if ((sb.st_mode & S_IFMT) == S_IFDIR) {
		snprintf(message, len, "%s is a directory", filename);
		return(message);
	}

	return (NULL);
}
