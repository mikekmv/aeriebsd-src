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
static char sccsid[] = "@(#)n6.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)ntab.c	4.2 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "tdef.h"

/*
troff6.c

width functions, sizes and fonts
*/

int fontlab[] = {'R','I','B','S',0};

int
width(int c)
{
	int i, j, k;

	j = c;
	k = 0;
	if (j & MOT) {
		if (j & VMOT)
			goto rtn;
		k = j & ~MOTV;
		if (j & NMOT)
			k = -k;
		goto rtn;
	}
	if ((i = (j & CMASK)) == 010) {
		k = -widthp;
		goto rtn;
	}
	if (i == PRESC)
		i = eschar;
	if ((i == ohc) || (i >= 0370))
		goto rtn;
	if (j & ZBIT)
		goto rtn;
	i = trtab[i] & BMASK;
	if (i < 040)
		goto rtn;
	if (t.codetab[i-32])
		k = (*(t.codetab[i-32]) & 0177) * t.Char;
	else
		k = 0;
	widthp = k;
rtn:
	return(k);
}

int
setch(void)
{
	extern const int chtab[];
	const int *j;
	int i, k;

	if ((i = getrq()) == 0)
		return(0);
	for (j = chtab; *j != i; j++)
		if (*(j++) == 0)
			return(0);
	k = *(++j) | chbits;
	return (k);
}

int
find(int i, int j[])
{
	int k;

	if (((k = i-'0') >= 1) && (k <= 4) && (k != smnt))
		return(--k);
	for (k = 0; j[k] != i; k++)
		if (j[k] == 0)
			return (-1);
	return (k);
}

void
mchbits(void)
{
	chbits = (((pts)<<2) | font) << (BYTE + 1);
	sps = width(' ' | chbits);
}

void
setps(void)
{
	int i, j;

	if (((i = getch() & CMASK) == '+' || i == '-') &&
	    (j = (ch = getch() & CMASK) - '0') >= 0 && j <= 9) {
		ch = 0;
		return;
	}
	if ((i -= '0') == 0)
		return;
	if ((i > 0) && (i <= 9)) {
		if (i <= 3 && (j = (ch = getch() & CMASK) - '0') >= 0 &&
		    j <= 9) {
			i = 10 * i + j;
			ch = 0;
		}
	}
}

void
caseft(void)
{
	skip();
	setfont(1);
}

void
setfont(int a)
{
	int i, j;

	if (a)
		i = getrq();
	else
		i = getsn();
	if(!i || (i == 'P')) {
		j = font1;
		goto s0;
	}
	if(i == 'S')
		return;
	if((j = find(i,fontlab)) == -1)
		return;
s0:
	font1 = font;
	font = j;
	mchbits();
}

void
setwd(void)
{
	int i, base, wid, delim, em, k;
	int savlevel, savhp, savfont, savfont1;
	int *savpinchar, *p, *q, tempinchar[LNSIZE];	/* XXX */

	base = v.st = v.sb = wid = v.ct = 0;
	if ((delim = getch() & CMASK) & MOT)
		return;
	savhp = v.hp;
	savpinchar = pinchar;	/* XXX */
	for (p = inchar, q = tempinchar; p < pinchar; )	/* XXX */
		*q++ = *p++;	/* XXX */
	pinchar = inchar;	/* XXX */
	savlevel = level;
	v.hp = level = 0;
	savfont = font;
	savfont1 = font1;
	setwdf++;
	while ((((i = getch()) & CMASK) != delim) && !nlflg) {
		wid += width(i);
		if (!(i & MOT)) {
			em = 2 * t.Halfline;
		} else if (i & VMOT) {
			k = i & ~MOTV;
			if (i & NMOT)
				k = -k;
			base -= k;
			em = 0;
		} else
			continue;
		if (base < v.sb)
			v.sb = base;
		if ((k=base + em) > v.st)
			v.st = k;
	}
	nform = 0;
	setn1(wid);
	v.hp = savhp;
	pinchar = savpinchar;	/* XXX */
	for (p = inchar, q = tempinchar; p < pinchar; )	/* XXX */
		*p++ = *q++;	/* XXX */
	level = savlevel;
	font = savfont;
	font1 = savfont1;
	mchbits();
	setwdf = 0;
}

int
vmot(void)
{
	dfact = lss;
	vflag++;
	return (mot());
}

int
hmot(void)
{
	dfact = EM;
	return (mot());
}

int
mot(void)
{
	int i, j;

	j = HOR;
	getch();	/* eat delim */
	if ((i = atoi0())) {
		if (vflag)
			j = VERT;
		i = makem(quant(i, j));
	}
	getch();
	vflag = 0;
	dfact = 1;
	return (i);
}

int
sethl(int k)
{
	int i;

	i = t.Halfline;
	if (k == 'u')
		i = -i;
	else if (k == 'r')
		i = -2*i;
	vflag++;
	i = makem(i);
	vflag = 0;
	return (i);
}

int
makem(int i)
{
	int j;

	if ((j = i) < 0)
		j = -j;
	j = (j & ~MOTV) | MOT;
	if (i < 0)
		j |= NMOT;
	if (vflag)
		j |= VMOT;
	return (j);
}

void
casefp(void)
{
	int i, j;

	skip();
	if (((i = (getch() & CMASK) - '0' -1) < 0) || (i >3))
		return;
	if (skip() || !(j = getrq()))
		return;
	fontlab[i] = j;
}

void
casevs(void)
{
	int i;

	skip();
	vflag++;
	dfact = INCH; /*default scaling is points!*/
	dfactd = 72;
	res = VERT;
	i = inumb(&lss);
	if (nonumb)
		i = lss1;
	if (i < VERT)
		i = VERT;
	lss1 = lss;
	lss = i;
}

int
xlss(void)
{
	int i, j;

	getch();
	dfact = lss;
	i = quant(atoi0(), VERT);
	dfact = 1;
	getch();
	if ((j = i) < 0)
		j = -j;
	ch0 = ((j & 03700) << 3) | HX;
	if (i < 0)
		ch0 |= 040000;
	return (((j & 077) << 9) | LX);
}

void
casefz(void)
{
}

void
caseps(void)
{
}

void
caselg(void)
{
}

void
casecs(void)
{
}

void
casebd(void)
{
}

void
casess(void)
{
}

int
getlg(int i)
{
	return (i);
}

#ifdef NROFF
/*
character name tables
modified for BTL special font version 4
and Commercial II
*/

const int chtab [] = {
PAIR('h','y'), 0200,	/*hyphen*/
PAIR('b','u'), 0201,	/*bullet*/
PAIR('s','q'), 0202,	/*square*/
PAIR('e','m'), 0203,	/*3/4em*/
PAIR('r','u'), 0204,	/*rule*/
PAIR('1','4'), 0205,	/*1/4*/
PAIR('1','2'), 0206,	/*1/2*/
PAIR('3','4'), 0207,	/*3/4*/
PAIR('m','i'), 0302,	/*equation minus*/
PAIR('f','i'), 0211,	/*fi*/
PAIR('f','l'), 0212,	/*fl*/
PAIR('f','f'), 0213,	/*ff*/
PAIR('F','i'), 0214,	/*ffi*/
PAIR('F','l'), 0215,	/*ffl*/
PAIR('d','e'), 0216,	/*degree*/
PAIR('d','g'), 0217,	/*dagger*/
PAIR('s','c'), 0220,	/*section*/
PAIR('f','m'), 0221,	/*foot mark*/
PAIR('a','a'), 0222,	/*acute accent*/
PAIR('g','a'), 0223,	/*grave accent*/
PAIR('u','l'), 0224,	/*underrule*/
PAIR('s','l'), 0225,	/*slash (longer)*/
PAIR('*','a'), 0230,	/*alpha*/
PAIR('*','b'), 0231,	/*beta*/
PAIR('*','g'), 0232,	/*gamma*/
PAIR('*','d'), 0233,	/*delta*/
PAIR('*','e'), 0234,	/*epsilon*/
PAIR('*','z'), 0235,	/*zeta*/
PAIR('*','y'), 0236,	/*eta*/
PAIR('*','h'), 0237,	/*theta*/
PAIR('*','i'), 0240,	/*iota*/
PAIR('*','k'), 0241,	/*kappa*/
PAIR('*','l'), 0242,	/*lambda*/
PAIR('*','m'), 0243,	/*mu*/
PAIR('*','n'), 0244,	/*nu*/
PAIR('*','c'), 0245,	/*xi*/
PAIR('*','o'), 0246,	/*omicron*/
PAIR('*','p'), 0247,	/*pi*/
PAIR('*','r'), 0250,	/*rho*/
PAIR('*','s'), 0251,	/*sigma*/
PAIR('*','t'), 0252,	/*tau*/
PAIR('*','u'), 0253,	/*upsilon*/
PAIR('*','f'), 0254,	/*phi*/
PAIR('*','x'), 0255,	/*chi*/
PAIR('*','q'), 0256,	/*psi*/
PAIR('*','w'), 0257,	/*omega*/
PAIR('*','A'), 0101,	/*Alpha*/
PAIR('*','B'), 0102,	/*Beta*/
PAIR('*','G'), 0260,	/*Gamma*/
PAIR('*','D'), 0261,	/*Delta*/
PAIR('*','E'), 0105,	/*Epsilon*/
PAIR('*','Z'), 0132,	/*Zeta*/
PAIR('*','Y'), 0110,	/*Eta*/
PAIR('*','H'), 0262,	/*Theta*/
PAIR('*','I'), 0111,	/*Iota*/
PAIR('*','K'), 0113,	/*Kappa*/
PAIR('*','L'), 0263,	/*Lambda*/
PAIR('*','M'), 0115,	/*Mu*/
PAIR('*','N'), 0116,	/*Nu*/
PAIR('*','C'), 0264,	/*Xi*/
PAIR('*','O'), 0117,	/*Omicron*/
PAIR('*','P'), 0265,	/*Pi*/
PAIR('*','R'), 0120,	/*Rho*/
PAIR('*','S'), 0266,	/*Sigma*/
PAIR('*','T'), 0124,	/*Tau*/
PAIR('*','U'), 0270,	/*Upsilon*/
PAIR('*','F'), 0271,	/*Phi*/
PAIR('*','X'), 0130,	/*Chi*/
PAIR('*','Q'), 0272,	/*Psi*/
PAIR('*','W'), 0273,	/*Omega*/
PAIR('s','r'), 0274,	/*square root*/
PAIR('t','s'), 0275,	/*terminal sigma*/
PAIR('r','n'), 0276,	/*root en*/
PAIR('>','='), 0277,	/*>=*/
PAIR('<','='), 0300,	/*<=*/
PAIR('=','='), 0301,	/*identically equal*/
PAIR('~','='), 0303,	/*approx =*/
PAIR('a','p'), 0304,	/*approximates*/
PAIR('!','='), 0305,	/*not equal*/
PAIR('-','>'), 0306,	/*right arrow*/
PAIR('<','-'), 0307,	/*left arrow*/
PAIR('u','a'), 0310,	/*up arrow*/
PAIR('d','a'), 0311,	/*down arrow*/
PAIR('e','q'), 0312,	/*equation equal*/
PAIR('m','u'), 0313,	/*multiply*/
PAIR('d','i'), 0314,	/*divide*/
PAIR('+','-'), 0315,	/*plus-minus*/
PAIR('c','u'), 0316,	/*cup (union)*/
PAIR('c','a'), 0317,	/*cap (intersection)*/
PAIR('s','b'), 0320,	/*subset of*/
PAIR('s','p'), 0321,	/*superset of*/
PAIR('i','b'), 0322,	/*improper subset*/
PAIR('i','p'), 0323,	/*  " superset*/
PAIR('i','f'), 0324,	/*infinity*/
PAIR('p','d'), 0325,	/*partial derivative*/
PAIR('g','r'), 0326,	/*gradient*/
PAIR('n','o'), 0327,	/*not*/
PAIR('i','s'), 0330,	/*integral sign*/
PAIR('p','t'), 0331,	/*proportional to*/
PAIR('e','s'), 0332,	/*empty set*/
PAIR('m','o'), 0333,	/*member of*/
PAIR('p','l'), 0334,	/*equation plus*/
PAIR('r','g'), 0335,	/*registered*/
PAIR('c','o'), 0336,	/*copyright*/
PAIR('b','r'), 0337,	/*box vert rule*/
PAIR('c','t'), 0340,	/*cent sign*/
PAIR('d','d'), 0341,	/*dbl dagger*/
PAIR('r','h'), 0342,	/*right hand*/
PAIR('l','h'), 0343,	/*left hand*/
PAIR('*','*'), 0344,	/*math * */
PAIR('b','s'), 0345,	/*bell system sign*/
PAIR('o','r'), 0346,	/*or*/
PAIR('c','i'), 0347,	/*circle*/
PAIR('l','t'), 0350,	/*left top (of big curly)*/
PAIR('l','b'), 0351,	/*left bottom*/
PAIR('r','t'), 0352,	/*right top*/
PAIR('r','b'), 0353,	/*right bot*/
PAIR('l','k'), 0354,	/*left center of big curly bracket*/
PAIR('r','k'), 0355,	/*right center of big curly bracket*/
PAIR('b','v'), 0356,	/*bold vertical*/
PAIR('l','f'), 0357,	/*left floor (left bot of big sq bract)*/
PAIR('r','f'), 0360,	/*right floor (rb of ")*/
PAIR('l','c'), 0361,	/*left ceiling (lt of ")*/
PAIR('r','c'), 0362,	/*right ceiling (rt of ")*/
PAIR('8','8'), 0363,	/*activate 8bit passing*/
PAIR('8','0'), 0364,	/*8bit: bit is 0*/
PAIR('8','1'), 0365,	/*8bit: bit is 1*/
0,0};
#endif
