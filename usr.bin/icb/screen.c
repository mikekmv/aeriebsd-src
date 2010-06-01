/*
 * Copyright (c) 2009 Konrad Merz
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

#include <sys/ioctl.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <term.h>

#include "screen.h"

/* 
 * from more: screen.c to provide terminal functions
 */

/*
 * Strings passed to putp() to do various terminal functions.
 */
char
	*sc_lower_left,		/* Cursor to last line, first column */
	*sc_move,		/* General cursor positioning */
	*sc_del_line,		/* move cursor to beginning of line */
	*sc_eol_clear,		/* Clear to end of line */
        *sc_backspace,          /* Backspace cursor */
	*sc_init,		/* Startup terminal initialization */
	*sc_deinit,		/* Exit terminal de-intialization */
	*sc_curs_left,		/* move cursor one left */
	*sc_curs_down,		/* move cursor one line down */
	*sc_curs_right,		/* move cursor one right */
	*sc_s_keypad,		/* Enter Keypad transmission */
	*sc_e_keypad;		/* Exit Keypad transmission */

int auto_wrap;		/* Terminal does \r\n when write past margin */
int ignaw;		/* Terminal ignores \n immediately after wrap */
				/* The user's erase and line-kill chars */
int retain_below;	/* Terminal retains text below the screen */
int erase_char, kill_char, werase_char;
int sc_width, sc_height = -1;/* Height & width of screen */
int sc_window = -1;	/* window size for forward and backward */

/*
 * These two variables are sometimes defined in,
 * and needed by, the termcap library.
 * It may be necessary on some systems to declare them extern here.
 */
short ospeed;	/* Terminal output baud rate */
char PC;	/* Pad character */

#define MAX_CMDTABLE	256
#define	MAX_CMDLEN	16

char *cmdendtable = NULL;
char kbuf[MAX_CMDLEN+1];
char *kp = kbuf;

char cmdtable[MAX_CMDTABLE] = {
	A_TABLE_END,
};

int cmd_search(const char *, const char *);
void noprefix(void);
void add_cmdtable(char *, int);
void special_key_init(void);

/*
 * Change terminal to "raw mode", or restore to "normal" mode.
 * "Raw mode" means 
 *	1. An outstanding read will complete on receipt of a single
 *	   keystroke.
 *	2. Input is not echoed.  
 *	3. On output, \n is mapped to \r\n.
 *	4. \t is NOT expanded into spaces.
 *	5. Signal-causing characters such as ctrl-C (interrupt),
 *	   etc. are NOT disabled.
 * It doesn't matter whether an input \n is mapped to \r, or vice versa.
 */
void
raw_mode(int on)
{
	struct termios s;
	static struct termios save_term;

	if (on) {
		/*
		 * Get terminal modes.
		 */
		tcgetattr(2, &s);

		/*
		 * Save modes and set certain variables dependent
		 * on modes.
		 */
		save_term = s;
		ospeed = cfgetospeed(&s);
		erase_char = s.c_cc[VERASE];
		kill_char = s.c_cc[VKILL];
		werase_char = s.c_cc[VWERASE];

		/*
		 * Set the modes to the way we want them.
		 */
		s.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
		s.c_oflag |=  (OPOST|ONLCR);
		s.c_cc[VMIN] = 1;
		s.c_cc[VTIME] = 0;
		setvbuf(stdout, NULL, _IOFBF, 0);
	} else {
		/*
		 * Restore saved modes.
		 */
		s = save_term;
	}
	tcsetattr(2, TCSADRAIN, &s);
}

/*
 * Get terminal capabilities via termcap.
 */
void
get_term(void)
{
	static char sbuf[1024];
	struct winsize w;
	char *sp, *term;
	int hard;

	/*
	 * Find out what kind of terminal this is.
	 */
 	if ((term = getenv("TERM")) == NULL)
 		term = "dumb";

 	if (tgetent(NULL, term) <= 0)
		err(1,"tgetent");

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

	auto_wrap = tgetflag("am");
	ignaw = tgetflag("xn");
	retain_below = tgetflag("db");

	/*
	 * Get various string-valued capabilities.
	 */
	sp = sbuf;

	sc_eol_clear = tgetstr("ce", &sp);
	if (hard || sc_eol_clear == NULL || *sc_eol_clear == '\0')
		sc_eol_clear = "";
	
	if ((sc_curs_left = tgetstr("le", &sp)) == NULL || 
	    (sc_curs_down = tgetstr("do", &sp)) == NULL) {
		fprintf(stderr, "no left no down\n");
		sc_curs_left = "";
		sc_curs_down = "";
	}

	sc_curs_right = tgetstr("nd", &sp);
	if (hard || sc_curs_right == NULL || *sc_curs_right == '\0') {
		fprintf(stderr, "no right\n");
		sc_curs_right = "";
	}
	
	sc_move = tgetstr("cm", &sp);
	if (hard || sc_move == NULL || *sc_move == '\0') {
		/*
		 * This is not an error here, because we don't 
		 * always need sc_move.
		 * We need it only if we don't have home or lower-left.
		 */
		sc_move = "";
	}

	sc_del_line = tgetstr("dl", &sp);
	if (sc_del_line == NULL || *sc_del_line == '\0') {
		fprintf(stderr, "shit again\n");
		sc_del_line = "";
	}

	if (!(sc_lower_left = tgetstr("ll", &sp)) &&
	    (sc_lower_left = tgetstr("cm", &sp)))
		sc_lower_left = tgoto(sc_lower_left, 0, sc_height);
	if (hard || sc_lower_left == NULL || *sc_lower_left == '\0') {
		if (*sc_move == '\0')
			sc_lower_left = "\r";
		else {
			/*
			 * No "lower-left" string, 
			 * but we can use "move(0,last-line)".
			 */
			strlcat(sbuf, tgoto(sc_move, 0, sc_height-1),
			    sizeof sbuf);
			sc_lower_left = sp;
			sp += strlen(sp) + 1;
		}
	}

	if (tgetflag("bs"))
		sc_backspace = "\b";
	else {
		sc_backspace = tgetstr("bc", &sp);
		if (sc_backspace == NULL || *sc_backspace == '\0')  
			sc_backspace = "\b";
	}

	sc_s_keypad = tgetstr("ks", &sp);
	if (sc_s_keypad == NULL)
		sc_s_keypad = "";
	sc_e_keypad = tgetstr("ke", &sp);
	if (sc_e_keypad == NULL)
		sc_e_keypad = "";
}

void
init(void)
{
	tputs(sc_s_keypad, sc_height, putchar);
	special_key_init();
}

void
deinit(void)
{
	tputs(sc_e_keypad, sc_height, putchar); 
}

void
special_key_init(void)
{
	static char sbuf[1024];
	char *sp = sbuf;
	char *res = NULL;

	/* LEFT */
        res = tgetstr("kl", &sp);
	add_cmdtable(res, A_CUR_LEFT);
	
	/* RIGHT */
        res = tgetstr("kr", &sp);
	add_cmdtable(res, A_CUR_RIGHT);
	
	/* RIGHT */
        res = tgetstr("kb", &sp);
	add_cmdtable(res, A_BACKSPACE);
}

void
put_line(char *msg)
{
	int c, column = 0;
	char *p;

	if (msg == NULL)
		return;

	for (p = msg; *p != '\0'; p++)
		switch(c = *p) {
		case '\b':
			putbs();
			column--;
			break;
		default:
			if (c & 0200) {
				putchar('^');
				putchar(c & 0177);
				column += 2;
			} else {
				putchar(c);
				column++;
			}
		}
	fflush(stdout);
}

void
noprefix(void)
{
	kp = kbuf;
}

int
decode_input(char c)
{
	int action = A_INVALID;

	*kp++ = c;
	*kp = '\0';

	action = cmd_search(cmdtable, cmdendtable);
	if (action != A_PREFIX)
		noprefix();

	return action;
}

/*
 * Add a new command to the table
 */
void
add_cmdtable(char *cmd, int op)
{
	if (cmd == NULL)
		return;
	if (cmdendtable == NULL)
		for (cmdendtable = cmdtable;
		    *cmdendtable != A_TABLE_END;
		    cmdendtable++);
	/*
	 * If the command <cmd> + 0 + <op> doesn't fit
	 * anymore skip it
	 */
	if ((unsigned)MAX_CMDTABLE - (cmdendtable - cmdtable)
	    < strlen(cmd) + 2)
		return;

	for (; *cmd != '\0'; cmd++)
		*(cmdendtable++) = *cmd;
	*(cmdendtable++) = 0;
	*(cmdendtable++) = op;
}

/* 
 * copied from more: decode.c
 * Search a command table for the current command string (in kbuf).
 */
int
cmd_search(const char *table, const char *endtable)
{
	const char *p;
	char *q;

	for (p = table, q = kbuf;  p < endtable;  p++, q++) {
		if (*p == *q) {
			/*
			 * Current characters match.
			 * If we're at the end of the string, we've
			 * found it.
			 * Return the action code,
			 * which is the character
			 * after the null at the end of the string
			 * in the command table.
			 */
			if (*p == '\0')
				return(p[1]);
		}
		else if (*q == '\0') {
			/*
			 * Hit the end of the user's command,
			 * but not the end of the string in the
			 * command table.
			 * The user's command is incomplete.
			 */
			return(A_PREFIX);
		} else {
			/*
			 * Not a match.
			 * Skip ahead to the next command in the
			 * command table, and reset the pointer
			 * to the user's command.
			 */
			while (*p++ != '\0');
			q = kbuf-1;
		}
	}
	/*
	 * No match found in the entire command table.
	 */
	return(A_INVALID);
}
