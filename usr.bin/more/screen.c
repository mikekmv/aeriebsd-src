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
static char sccsid[] = "@(#)screen.c	8.2 (Berkeley) 4/20/94";
#else
static const char rcsid[] =
    "$ABSD: screen.c,v 1.3 2009/09/17 12:36:49 mickey Exp $";
#endif
#endif /* not lint */

/*
 * Routines which deal with the characteristics of the terminal.
 * Uses termcap to be as terminal-independent as possible.
 *
 * {{ Someday this should be rewritten to use curses. }}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <term.h>
#include <err.h>

#include "less.h"

/*
 * Strings passed to putp() to do various terminal functions.
 */
char
	*sc_pad,		/* Pad string */
	*sc_home,		/* Cursor home */
	*sc_addline,		/* Add line, scroll down following lines */
	*sc_lower_left,		/* Cursor to last line, first column */
	*sc_move,		/* General cursor positioning */
	*sc_clear,		/* Clear screen */
	*sc_eol_clear,		/* Clear to end of line */
	*sc_s_in,		/* Enter standout (highlighted) mode */
	*sc_s_out,		/* Exit standout mode */
	*sc_u_in,		/* Enter underline mode */
	*sc_u_out,		/* Exit underline mode */
	*sc_b_in,		/* Enter bold mode */
	*sc_b_out,		/* Exit bold mode */
	*sc_backspace,		/* Backspace cursor */
	*sc_init,		/* Startup terminal initialization */
	*sc_deinit,		/* Exit terminal de-intialization */
	*sc_s_keypad,		/* Enter Keypad transmission */
	*sc_e_keypad;		/* Exit Keypad transmission */

int auto_wrap;			/* Terminal does \r\n when write past margin */
int ignaw;			/* Terminal ignores \n immediately after wrap */
				/* The user's erase and line-kill chars */
int retain_below;		/* Terminal retains text below the screen */
int erase_char, kill_char, werase_char;
int sc_width, sc_height = -1;	/* Height & width of screen */
int sc_window = -1;		/* window size for forward and backward */
int bo_width, be_width;		/* Printing width of boldface sequences */
int ul_width, ue_width;		/* Printing width of underline sequences */
int so_width, se_width;		/* Printing width of standout sequences */

/*
 * These two variables are sometimes defined in,
 * and needed by, the termcap library.
 * It may be necessary on some systems to declare them extern here.
 */
extern short ospeed;	/* Terminal output baud rate */
extern char PC;		/* Pad character */

/*
 * Change terminal to "raw mode", or restore to "normal" mode.
 * "Raw mode" means 
 *	1. An outstanding read will complete on receipt of a single keystroke.
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
		 * Save modes and set certain variables dependent on modes.
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
	 * Assumes termcap variable "sg" is the printing width of
	 * the standout sequence, the end standout sequence,
	 * the underline sequence, the end underline sequence,
	 * the boldface sequence, and the end boldface sequence.
	 */
	if ((so_width = tgetnum("sg")) < 0)
		so_width = 0;
	be_width = bo_width = ue_width = ul_width = se_width = so_width;

	/*
	 * Get various string-valued capabilities.
	 */
	sp = sbuf;

	sc_pad = tgetstr("pc", &sp);
	if (sc_pad != NULL)
		PC = *sc_pad;

#if 0
	sc_init = tgetstr("ti", &sp);
	if (sc_init == NULL)
#endif
		sc_init = "";

#if 0
	sc_deinit= tgetstr("te", &sp);
	if (sc_deinit == NULL)
#endif
		sc_deinit = "";

	sc_eol_clear = tgetstr("ce", &sp);
	if (hard || sc_eol_clear == NULL || *sc_eol_clear == '\0')
		sc_eol_clear = "";

	sc_clear = tgetstr("cl", &sp);
	if (hard || sc_clear == NULL || *sc_clear == '\0')
		sc_clear = "\n\n";

	sc_move = tgetstr("cm", &sp);
	if (hard || sc_move == NULL || *sc_move == '\0') {
		/*
		 * This is not an error here, because we don't 
		 * always need sc_move.
		 * We need it only if we don't have home or lower-left.
		 */
		sc_move = "";
	}

	sc_s_in = tgetstr("so", &sp);
	if (hard || sc_s_in == NULL)
		sc_s_in = "";

	sc_s_out = tgetstr("se", &sp);
	if (hard || sc_s_out == NULL)
		sc_s_out = "";

	sc_u_in = tgetstr("us", &sp);
	if (hard || sc_u_in == NULL)
		sc_u_in = sc_s_in;

	sc_u_out = tgetstr("ue", &sp);
	if (hard || sc_u_out == NULL)
		sc_u_out = sc_s_out;

	sc_b_in = tgetstr("md", &sp);
	if (hard || sc_b_in == NULL) {
		sc_b_in = sc_s_in;
		sc_b_out = sc_s_out;
	} else {
		sc_b_out = tgetstr("me", &sp);
		if (hard || sc_b_out == NULL)
			sc_b_out = "";
	}

	sc_home = tgetstr("ho", &sp);
	if (hard || sc_home == NULL || *sc_home == '\0') {
		if (*sc_move == '\0') {
			/*
			 * This last resort for sc_home is supposed to
			 * be an up-arrow suggesting moving to the 
			 * top of the "virtual screen". (The one in
			 * your imagination as you try to use this on
			 * a hard copy terminal.)
			 */
			sc_home = "|\b^";
		} else {
			/* 
			 * No "home" string,
			 * but we can use "move(0,0)".
			 */
			strlcat(sbuf, tgoto(sc_move, 0, 0), sizeof sbuf);
			sc_home = sp;
			sp += strlen(sp) + 1;
		}
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

	/*
	 * To add a line at top of screen and scroll the display down,
	 * we use "al" (add line) or "sr" (scroll reverse).
	 */
	if ((sc_addline = tgetstr("al", &sp)) == NULL || 
		 *sc_addline == '\0')
		sc_addline = tgetstr("sr", &sp);

	if (hard || sc_addline == NULL || *sc_addline == '\0') {
		sc_addline = "";
		/* Force repaint on any backward movement */
		back_scroll = 0;
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


/*
 * Below are the functions which perform all the 
 * terminal-specific screen manipulation.
 */

int short_file;				/* if file less than a screen */
void
lower_left(void)
{
	if (short_file) {
		putchar('\r');
		fflush(stdout);
	} else
		putp(sc_lower_left);
}

/*
 * Erase the character to the left of the cursor 
 * and move the cursor left.
 */
void
backspace(void)
{
	/* 
	 * Try to erase the previous character by overstriking with a space.
	 */
	putp(sc_backspace);
	putchar(' ');
	putp(sc_backspace);
}

void
special_key_init(void)
{
	static char sbuf[1024];
	char *sp = sbuf;
	char *res = NULL;

	/* Arrow up */
	res = tgetstr("ku", &sp);
	add_cmdtable(res, A_B_LINE);

	/* Arrow down */
	res = tgetstr("kd", &sp);
	add_cmdtable(res, A_F_LINE);

	/* Page up */
	res = tgetstr("kP", &sp);
	add_cmdtable(res, A_B_SCREEN);

	/* Page down */
	res = tgetstr("kN", &sp);
	add_cmdtable(res, A_F_SCREEN);
}

void
init(void)
{
	tputs(sc_init, sc_height, putchar);
	tputs(sc_s_keypad, sc_height, putchar);
	special_key_init();
}

void
deinit(void)
{
	tputs(sc_init, sc_height, putchar);
	tputs(sc_s_keypad, sc_height, putchar);
}
