/*
 * Copyright (c) 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
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
static char sccsid[] = "@(#)stoc.c	8.1 (Berkeley) 7/24/94";
#else
static char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "gomoku.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char	*letters	= "<ABCDEFGHJKLMNOPQRST>";

struct mvstr {
	int	m_code;
	char	*m_text;
};
static	struct	mvstr	mv[] = {
	{RESIGN,	"resign" },
	{RESIGN,	"quit" },
	{SAVE,		"save" },
	{-1,		0}
};

/*
 * Turn the spot number form of a move into the character form.
 */
char *
stoc(s)
	int s;
{
	static char buf[32];
	int i;

	for (i = 0; mv[i].m_code >= 0; i++)
		if (s == mv[i].m_code)
			return(mv[i].m_text);
	snprintf(buf, sizeof buf, "%c%d", letters[s % BSZ1], s / BSZ1);
	return(buf);
}

/*
 * Turn the character form of a move into the spot number form.
 */
int
ctos(mp)
	char *mp;
{
	int i;

	for (i = 0; mv[i].m_code >= 0; i++)
		if (strcmp(mp, mv[i].m_text) == 0)
			return(mv[i].m_code);
	if (!isalpha(mp[0]))
		return(ILLEGAL);
	i = atoi(&mp[1]);
	if (i < 1 || i > 19)
		return(ILLEGAL);
	return(PT(lton(mp[0]), i));
}

/*
 * Turn a letter into a number.
 */
int
lton(c)
	int c;
{
	int i;

	if (islower(c))
		c = toupper(c);
	for (i = 1; i <= BSZ && letters[i] != c; i++)
		;
	return(i);
}
