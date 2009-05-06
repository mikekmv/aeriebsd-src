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
static char sccsid[] = "@(#)output.c	8.2 (Berkeley) 4/27/95";
static char sccsid[] = "@(#)ttyin.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] =
    "$ABSD$";
#endif
#endif /* not lint */

/*
 * High level routines dealing with the output to the screen.
 * Routines dealing with getting input from the keyboard (i.e. from the user).
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <term.h>

#include "less.h"

int errmsgs;	/* Count of messages displayed by error() */

/* display the line which is in the line buffer. */
void
put_line(void)
{
	char *p;
	int c, column;

	if (sigs) {
		/*
		 * Don't output if a signal is pending.
		 */
		screen_trashed = 1;
		return;
	}

	if (line == NULL)
		line = "";

	column = 0;
	for (p = line;  *p != '\0';  p++)
		switch (c = *p) {
		case UL_CHAR:
			ul_enter();
			column += ul_width +1;
			break;
		case UE_CHAR:
			ul_exit();
			column += ue_width;
			break;
		case BO_CHAR:
			bo_enter();
			column += bo_width +1;
			break;
		case BE_CHAR:
			bo_exit();
			column += be_width;
			break;
		case '\t':
			do {
				putchar(' ');
				column++;
			} while ((column % tabstop) != 0);
			break;
		case '\b':
			putbs();
			column--;
			break;
		default:
			if (c & 0200) {
				/*
				 * Control characters arrive here as the
				 * normal character [CARAT_CHAR(c)] with
				 * the 0200 bit set.  See pappend().
				 */
				putchar('^');
				putchar(c & 0177);
				column += 2;
			} else {
				putchar(c);
				column++;
			}
		}

	if (column < sc_width || !auto_wrap || ignaw)
		putchar('\n');
}

int cmdstack;
static const char return_to_continue[] = "(press RETURN)";

/*
 * Output a message in the lower left corner of the screen
 * and wait for carriage return.
 */
void
error(const char *s)
{
	int ch;

	++errmsgs;
	if (!any_display) {
		/*
		 * Nothing has been displayed yet.  Output this message on
		 * error output (file descriptor 2) and don't wait for a
		 * keystroke to continue.
		 *
		 * This has the desirable effect of producing all error
		 * messages on error output if standard output is directed
		 * to a file.  It also does the same if we never produce
		 * any real output; for example, if the input file(s) cannot
		 * be opened.  If we do eventually produce output, code in
		 * edit() makes sure these messages can be seen before they
		 * are overwritten or scrolled away.
		 */
		if (s != NULL)
			fprintf(stderr, "%s\n", s);
		return;
	}

	lower_left();
	clear_eol();
	so_enter();
	if (s != NULL)
		printf("%s  ", s);
	fputs(return_to_continue, stdout);
	so_exit();

	if ((ch = getchr()) != '\n') {
		if (ch == 'q')
			quit();
		cmdstack = ch;
	}
	lower_left();

	if ((s != NULL ? strlen(s) : 0) + sizeof(return_to_continue) + 
	    so_width + se_width + 1 > (size_t)sc_width)
		/*
		 * Printing the message has probably scrolled the screen.
		 * {{ Unless the terminal doesn't have auto margins,
		 *    in which case we just hammered on the right margin. }}
		 */
		repaint();
	fflush(stdout);
}

void
ierror(char *s)
{
	lower_left();
	clear_eol();
	so_enter();
	printf("%s ... (interrupt to abort)", s);
	so_exit();
	fflush(stdout);
}

/*
 * Get a character from the keyboard.
 */
int
getchr(void)
{
	char c;
	int result;

	do {
		result = iread(2, &c, 1);
		if (result == READ_INTR)
			return (READ_INTR);
		if (result < 0) {
			/*
			 * Don't call error() here,
			 * because error calls getchr!
			 */
			quit();
		}
	} while (result != 1);
	return (c & 0377);
}
