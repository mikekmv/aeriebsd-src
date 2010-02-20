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


#ifndef _SCREEN_H_
#define _SCREEN_H_

void raw_mode(int);
void get_term(void);
void init(void);
void deinit(void);
void put_line(char *);
int decode_input(char);

/* Actions of decode_input */
#define	A_TABLE_END	-1
#define	A_CHAR		 0
#define	A_CUR_LEFT	 1
#define A_CUR_RIGHT	 2
#define	A_BACKSPACE	 3
#define A_PREFIX	 4
#define A_INVALID	 5

/*
 * from more: screen.c
 * Strings passed to putp() to do various terminal functions.
 */
extern char
	*sc_lower_left,		/* Cursor to last line, first column */
	*sc_move,		/* General cursor positioning */
	*sc_del_line,		/* Delete line */
	*sc_eol_clear,		/* Clear to end of line */
        *sc_backspace,          /* Backspace cursor */
	*sc_init,		/* Startup terminal initialization */
	*sc_deinit,		/* Exit terminal de-intialization */
	*sc_curs_left,		/* move cursor one left */
	*sc_curs_down,		/* move cursor one down */
	*sc_curs_right,		/* move cursor one right */
	*sc_s_keypad,		/* Enter Keypad transmission */
	*sc_e_keypad;		/* Exit Keypad transmission */

extern	 int auto_wrap;		/* Terminal does \r\n when write past margin */
extern	 int ignaw;		/* Terminal ignores \n immediately after wrap */
				/* The user's erase and line-kill chars */
extern	 int retain_below;	/* Terminal retains text below the screen */
extern	 int erase_char, kill_char, werase_char;
extern	 int sc_width, sc_height;/* Height & width of screen */
extern	 int sc_window;	/* window size for forward and backward */

/*
 * These two variables are sometimes defined in,
 * and needed by, the termcap library.
 * It may be necessary on some systems to declare them extern here.
 */
extern	 short ospeed;	/* Terminal output baud rate */
extern	 char PC;	/* Pad character */

#define	clear()		putp(sc_clear);
#define	putbs()		putp(sc_backspace);
#define del_line()	putp(sc_del_line); 
#define curs_left()	putp(sc_curs_left);
#define curs_right()	putp(sc_curs_right);

#endif /* _SCREEN_H_ */
