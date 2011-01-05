/*
 * Copyright (C) Caldera International Inc. 2001-2002.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code and documentation must retain the above
 * copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * This product includes software developed or owned by Caldera
 * International, Inc.
 * 4. Neither the name of Caldera International, Inc. nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR ANY DIRECT,
 * INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*-
 * Copyright (c) 1991
 * The Regents of the University of California. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)n8.c	4.2 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)hytab.c	4.2 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "tdef.h"

/*
troff8.c

hyphenation
*/

char hbuf[NHEX];
char *nexth = hbuf;
int *hyend;
#define THRESH 160 /*digram goodness threshold*/
int thresh = THRESH;

int exword(void);
void digram(void);
int dilook(int, int, const char[26][13]);
int maplow(int);
int punct(int);
int suffix(void);

/*
 * Hyphenation digram tables
 */

const char	bxh[1][13] = {
	{ 0060,0000,0040,0000,0040,0000,0000,0040,0000,0000,0040,0000,0040 }
};

const char	hxx[26][13] = {
	{ 0006,0042,0041,0123,0021,0024,0063,0042,0002,0043,0021,0001,0022 },
	{ 0140,0000,0200,0003,0260,0006,0000,0160,0007,0000,0140,0000,0320 },
	{ 0220,0000,0160,0005,0240,0010,0000,0100,0006,0000,0200,0000,0320 },
	{ 0240,0000,0120,0003,0140,0000,0000,0240,0010,0000,0220,0000,0160 },
	{ 0042,0023,0041,0040,0040,0022,0043,0041,0030,0064,0021,0000,0041 },
	{ 0100,0000,0140,0000,0220,0006,0000,0140,0003,0000,0200,0000,0000 },
	{ 0200,0000,0120,0002,0220,0010,0000,0160,0006,0000,0140,0000,0320 },
	{ 0020,0000,0020,0000,0020,0000,0000,0020,0000,0000,0020,0000,0000 },
	{ 0043,0163,0065,0044,0022,0043,0104,0042,0061,0146,0061,0000,0007 },
	{ 0100,0000,0140,0000,0040,0000,0000,0100,0000,0000,0120,0000,0000 },
	{ 0140,0000,0040,0011,0060,0004,0001,0120,0003,0000,0140,0000,0040 },
	{ 0200,0000,0100,0000,0140,0000,0000,0140,0000,0000,0140,0000,0240 },
	{ 0200,0000,0140,0000,0160,0000,0000,0220,0000,0000,0140,0000,0240 },
	{ 0200,0000,0140,0000,0160,0000,0000,0220,0000,0000,0060,0000,0240 },
	{ 0021,0043,0041,0121,0040,0023,0042,0003,0142,0042,0061,0001,0022 },
	{ 0120,0000,0140,0010,0140,0010,0000,0140,0002,0000,0120,0000,0120 },
	{ 0000,0000,0000,0000,0360,0000,0000,0000,0000,0000,0160,0000,0000 },
	{ 0100,0000,0040,0005,0120,0000,0000,0100,0000,0000,0060,0000,0140 },
	{ 0140,0040,0100,0001,0240,0041,0000,0242,0000,0002,0140,0000,0100 },
	{ 0240,0000,0120,0002,0200,0000,0000,0320,0007,0000,0240,0000,0340 },
	{ 0101,0021,0041,0020,0040,0005,0042,0121,0002,0021,0201,0000,0020 },
	{ 0160,0000,0100,0000,0140,0000,0000,0160,0006,0000,0220,0000,0140 },
	{ 0140,0000,0020,0001,0020,0000,0000,0100,0001,0000,0300,0000,0000 },
	{ 0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000 },
	{ 0106,0041,0040,0147,0040,0000,0063,0041,0001,0102,0160,0002,0002 },
	{ 0300,0000,0040,0017,0140,0017,0000,0240,0000,0000,0140,0000,0120 },
};

const char	bxxh[26][13] = {
	{ 0005,0150,0153,0062,0062,0246,0152,0127,0146,0203,0310,0017,0206 },
	{ 0100,0000,0120,0000,0140,0000,0000,0100,0000,0000,0120,0000,0060 },
	{ 0100,0000,0040,0000,0060,0000,0000,0060,0000,0000,0220,0000,0040 },
	{ 0100,0000,0120,0000,0200,0000,0000,0100,0000,0000,0140,0000,0060 },
	{ 0043,0142,0046,0140,0062,0147,0210,0131,0046,0106,0246,0017,0111 },
	{ 0060,0000,0020,0000,0060,0000,0000,0040,0000,0000,0100,0000,0000 },
	{ 0060,0000,0040,0000,0040,0000,0000,0040,0000,0000,0100,0000,0040 },
	{ 0100,0000,0100,0000,0100,0000,0000,0040,0000,0000,0100,0000,0140 },
	{ 0066,0045,0145,0140,0000,0070,0377,0030,0130,0103,0003,0017,0006 },
	{ 0040,0000,0040,0000,0020,0000,0000,0040,0000,0000,0100,0000,0000 },
	{ 0200,0000,0020,0000,0140,0000,0000,0120,0000,0000,0120,0000,0040 },
	{ 0120,0000,0040,0000,0060,0000,0000,0060,0000,0000,0160,0000,0040 },
	{ 0120,0000,0040,0000,0120,0000,0000,0040,0000,0000,0160,0000,0040 },
	{ 0120,0000,0020,0000,0140,0000,0000,0120,0000,0000,0140,0000,0040 },
	{ 0051,0126,0150,0140,0060,0210,0146,0006,0006,0165,0003,0017,0244 },
	{ 0120,0000,0040,0000,0160,0000,0000,0140,0000,0000,0060,0000,0140 },
	{ 0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000 },
	{ 0140,0000,0140,0000,0060,0000,0000,0100,0000,0000,0140,0000,0020 },
	{ 0120,0000,0020,0000,0060,0000,0000,0060,0000,0000,0060,0000,0040 },
	{ 0140,0000,0020,0000,0100,0000,0000,0140,0000,0000,0140,0000,0020 },
	{ 0070,0125,0051,0162,0120,0105,0126,0104,0006,0044,0000,0017,0052 },
	{ 0140,0000,0020,0000,0140,0000,0000,0060,0000,0000,0060,0000,0040 },
	{ 0020,0000,0000,0000,0020,0000,0000,0000,0000,0000,0000,0000,0060 },
	{ 0140,0000,0160,0000,0200,0000,0000,0140,0000,0000,0000,0000,0240 },
	{ 0065,0042,0060,0200,0000,0210,0222,0146,0006,0204,0220,0012,0003 },
	{ 0240,0000,0020,0000,0120,0000,0000,0200,0000,0000,0200,0000,0240 },
};

const char	xhx[26][13] = {
	{ 0032,0146,0042,0107,0076,0102,0042,0146,0202,0050,0006,0000,0051 },
	{ 0036,0377,0057,0013,0057,0366,0377,0057,0001,0377,0057,0000,0040 },
	{ 0037,0377,0020,0000,0100,0022,0377,0057,0362,0116,0100,0000,0017 },
	{ 0057,0377,0057,0031,0137,0363,0377,0037,0362,0270,0077,0000,0117 },
	{ 0074,0142,0012,0236,0076,0125,0063,0165,0341,0046,0047,0000,0024 },
	{ 0020,0017,0075,0377,0040,0001,0377,0017,0001,0204,0020,0000,0040 },
	{ 0057,0017,0057,0340,0140,0362,0314,0117,0003,0302,0100,0000,0057 },
	{ 0057,0357,0077,0017,0100,0366,0314,0057,0342,0346,0037,0000,0060 },
	{ 0252,0145,0072,0157,0377,0165,0063,0066,0164,0050,0363,0000,0362 },
	{ 0000,0000,0020,0000,0020,0000,0000,0017,0000,0000,0020,0000,0000 },
	{ 0117,0017,0237,0377,0200,0354,0125,0110,0004,0257,0000,0000,0300 },
	{ 0057,0367,0054,0357,0157,0216,0314,0114,0217,0353,0053,0000,0057 },
	{ 0077,0213,0077,0077,0177,0317,0377,0114,0377,0352,0077,0000,0076 },
	{ 0077,0213,0077,0077,0157,0177,0377,0054,0377,0352,0117,0000,0075 },
	{ 0125,0230,0065,0216,0057,0066,0063,0047,0345,0126,0011,0000,0033 },
	{ 0057,0377,0051,0360,0120,0361,0273,0056,0001,0256,0057,0000,0060 },
	{ 0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000 },
	{ 0076,0310,0056,0310,0137,0174,0273,0055,0335,0266,0033,0000,0155 },
	{ 0077,0157,0057,0360,0057,0063,0042,0024,0077,0206,0020,0000,0040 },
	{ 0057,0037,0077,0360,0100,0365,0377,0037,0362,0176,0050,0000,0026 },
	{ 0167,0146,0042,0112,0077,0110,0062,0254,0366,0052,0377,0000,0163 },
	{ 0060,0000,0040,0000,0120,0000,0377,0060,0012,0000,0037,0000,0257 },
	{ 0037,0232,0157,0361,0040,0003,0125,0010,0001,0256,0000,0000,0340 },
	{ 0377,0377,0377,0377,0377,0377,0377,0377,0377,0377,0377,0017,0277 },
	{ 0253,0315,0257,0216,0377,0206,0146,0306,0371,0126,0232,0000,0004 },
	{ 0057,0012,0100,0360,0160,0360,0000,0040,0000,0017,0157,0000,0176 },
};

const char	xxh[26][13] = {
	{ 0045,0150,0154,0162,0042,0246,0210,0147,0152,0103,0230,0017,0206 },
	{ 0100,0000,0040,0000,0140,0000,0000,0100,0000,0021,0120,0017,0060 },
	{ 0100,0000,0040,0002,0140,0320,0000,0060,0000,0001,0220,0017,0040 },
	{ 0100,0001,0120,0001,0241,0000,0000,0100,0000,0020,0140,0017,0060 },
	{ 0023,0162,0046,0142,0022,0207,0210,0131,0052,0106,0250,0017,0110 },
	{ 0060,0000,0042,0000,0160,0000,0000,0040,0000,0212,0100,0017,0000 },
	{ 0140,0000,0040,0002,0140,0000,0000,0120,0000,0040,0120,0017,0040 },
	{ 0100,0000,0100,0000,0140,0001,0021,0140,0000,0046,0100,0017,0140 },
	{ 0066,0045,0025,0201,0020,0130,0146,0030,0130,0103,0025,0017,0006 },
	{ 0100,0000,0040,0000,0020,0000,0000,0040,0000,0000,0200,0017,0000 },
	{ 0200,0000,0020,0001,0140,0000,0000,0140,0000,0000,0120,0017,0040 },
	{ 0120,0026,0042,0020,0140,0161,0042,0143,0000,0022,0162,0017,0040 },
	{ 0121,0042,0060,0020,0140,0200,0000,0123,0000,0021,0220,0017,0041 },
	{ 0121,0042,0060,0120,0140,0200,0000,0123,0000,0021,0160,0017,0041 },
	{ 0051,0126,0150,0141,0060,0210,0146,0066,0026,0165,0026,0017,0247 },
	{ 0120,0000,0040,0003,0160,0000,0000,0140,0000,0021,0100,0017,0140 },
	{ 0000,0000,0000,0000,0200,0000,0000,0000,0000,0000,0000,0017,0000 },
	{ 0141,0023,0122,0040,0160,0143,0042,0142,0000,0047,0143,0017,0020 },
	{ 0120,0000,0040,0006,0140,0060,0000,0141,0000,0026,0100,0017,0040 },
	{ 0140,0000,0020,0007,0100,0000,0000,0140,0000,0001,0140,0017,0020 },
	{ 0110,0125,0051,0162,0120,0125,0127,0104,0006,0104,0000,0017,0052 },
	{ 0140,0000,0040,0000,0160,0000,0000,0140,0000,0000,0060,0017,0000 },
	{ 0040,0005,0020,0000,0040,0313,0231,0030,0000,0140,0000,0017,0056 },
	{ 0140,0000,0160,0000,0200,0000,0000,0140,0000,0000,0000,0017,0240 },
	{ 0065,0042,0060,0040,0000,0206,0231,0146,0006,0224,0220,0017,0004 },
	{ 0240,0000,0020,0000,0140,0000,0000,0220,0000,0000,0200,0017,0141 },
};

void
hyphen(int *wp)
{
	int *i, j;

	i = wp;
	while (punct(*i++))
		;
	if (!alph(*--i))
		return;
	wdstart = i++;
	while (alph(*i++))
		;
	hyend = wdend = --i-1;
	while (punct(*i++))
		;
	if (*--i)
		return;
	if ((wdend-wdstart-4) < 0)
		return;
	hyp = hyptr;
	*hyp = 0;
	hyoff = 2;
	if (!exword() && !suffix())
		digram();
	*hyp++ = 0;
	if (*hyptr) for(j = 1; j;) {
		j = 0;
		for(hyp = hyptr+1; *hyp != 0; hyp++) {
			if (*(hyp-1) > *hyp) {
				j++;
				i = *hyp;
				*hyp = *(hyp-1);
				*(hyp-1) = i;
			}
		}
	}
}

int
punct(int i)
{
	if (!i || alph(i))
		return(0);
	else
		return(1);
}

int
alph(int i)
{
	int j;

	j = i & CMASK;
	if (((j >= 'A') && (j <= 'Z')) || ((j >= 'a') && (j <= 'z')))
		return(1);
	else
		return(0);
}

void
caseht(void)
{
	thresh = THRESH;
	if (skip())
		return;
	noscale++;
	thresh = atoi0();
	noscale = 0;
}

void
casehw(void)
{
	int i, k;
	register char *j;

	k = 0;
	while(!skip()) {
		if ((j = nexth) >= (hbuf + NHEX - 2))
			goto full;
		for (;;) {
			if ((i = getch()) & MOT)
				continue;
			if (((i &= CMASK) == ' ') || (i == '\n')) {
				*j++ = 0;
				nexth = j;
				*j = 0;
				if (i == ' ')
					break;
				else
					return;
			}
			if (i == '-') {
				k = 0200;
				continue;
			}
			*j++ = maplow(i) | k;
			k = 0;
			if (j >= (hbuf + NHEX - 2))
				goto full;
		}
	}
	return;
full:
	prstr("Exception word list full.\n");
	*nexth = 0;
}

int
exword(void)
{
	int *w;
	char *e, *save;

	e = hbuf;
	while(1) {
		save = e;
		if (*e == 0)
			return(0);
		w = wdstart;
		while (*e && w <= hyend && (*e & 0177) == maplow(*w & CMASK)) {
			e++;
			w++;
		}
		if (!*e) {
			if (w - 1 == hyend ||
			   (w == wdend && maplow(*w & CMASK) == 's')) {
				w = wdstart;
				for (e = save; *e; e++) {
					if (*e & 0200)
						*hyp++ = w;
					if (hyp > (hyptr+NHYP-1))
						hyp = hyptr+NHYP-1;
					w++;
				}
				return (1);
			} else {
				e++;
				continue;
			}
		} else
			while (*e++)
				;
	}
}

int
suffix(void)
{
	extern const char *const suftab[];
	extern int *chkvow();
	int i, *w;
	const char *s, *s0;

again:
	if (!alph(i = *hyend & CMASK))
		return(0);
	if (i < 'a')
		i -= 'A'-'a';
	if ((s0 = suftab[i-'a']) == 0)
		return(0);
	for (;;) {
		if ((i = *s0 & 017) == 0)
			return(0);
		s = s0 + i - 1;
		w = hyend - 1;
		while(s > s0 && (w >= wdstart) && (*s & 0177) == maplow(*w)) {
			s--;
			w--;
		}
		if (s == s0)
			break;
		s0 += i;
	}
	s = s0 + i - 1;
	w = hyend;
	if (*s0 & 0200) goto mark;
	while(s > s0) {
		w--;
		if (*s-- & 0200) {
	mark:
			hyend = w - 1;
			if (*s0 & 0100)
				continue;
			if (!chkvow(w))
				return(0);
			*hyp++ = w;
		}
	}
	if (*s0 & 040)
		return(0);
	if (exword())
		return(1);
	goto again;
}

int
maplow(int i)
{
	if ((i &= CMASK) < 'a')
		i += 'a' - 'A';
	return(i);
}

int
vowel(int i)
{
	switch (maplow(i)) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'y':
		return(1);
	default:
		return(0);
	}
}

int *
chkvow(int *w)
{
	while (--w >= wdstart)
		if (vowel(*w & CMASK))
			return(w);
	return (0);
}

void
digram(void)
{
	int *w, val, *nhyend, *maxw, maxval;

	while ((w = chkvow(hyend + 1))) {
		maxw = hyend = w;
		if (!(w = chkvow(hyend)))
			return;
		nhyend = w;
		maxval = 0;
		w--;
		while ((++w < hyend) && (w < (wdend - 1))) {
			val = 1;
			if (w == wdstart)
				val *= dilook('a', *w, bxh);
			else if (w == wdstart + 1)
				val *= dilook(w[-1], *w, bxxh);
			else
				val *= dilook(w[-1], *w, xxh);
			val *= dilook(w[0], w[1], xhx);
			val *= dilook(w[1], w[2], hxx);
			if (val > maxval) {
				maxval = val;
				maxw = w + 1;
			}
		}
		hyend = nhyend;
		if (maxval > thresh)
			*hyp++ = maxw;
	}
}

int
dilook(int a, int b, const char t[26][13])
{
	int i, j;

	i = t[maplow(a)-'a'][(j = maplow(b)-'a')/2];
	if (!(j & 01))i >>= 4;
	return(i & 017);
}
