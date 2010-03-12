%{
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
static char sccsid[] = "@(#)e.y	4.3 (Berkeley) 3/1/93";
static char sccsid[] = "@(#)diacrit.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)eqnbox.c	4.3 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)font.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)fromto.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)funny.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)integral.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)mark.c	4.3 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)matrix.c	4.3 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)move.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)over.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)paren.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)pile.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)shift.c	4.4 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)size.c	4.3 (Berkeley) 4/17/91";
static char sccsid[] = "@(#)sqrt.c	4.5 (Berkeley) 3/1/93";
#else
static const char rcsid[] = "$ABSD: e.y,v 1.3 2009/11/03 14:32:48 mickey Exp $";
#endif
#endif /* not lint */

#include "e.h"
int	fromflg;

#define	YYSTYPE	long

void diacrit(int, int);
void eqnbox(int, int, int);
void setfont(char);
void font(int, int);
void fatbox(int);
void fromto(int, int, int);
void funny(int);
void integral(int, int, int);
void setintegral(void);
void mark(int);
void lineup(int);
void column(int, int);
void matrix(int);
void move(int, int, int);
void boverb(int, int);
void paren(int, int, int);
void brack(int, char *, char *, char *);
void lpile(int, int, int);
void bshiftb(int, int, int);
void shift(int);
void shift2(int, int, int);
void setsize(char *);
void size(int, int);
void eqsqrt(int);
%}
%term	CONTIG QTEXT SPACE THIN TAB
%term	MATRIX LCOL CCOL RCOL COL
%term	MARK LINEUP
%term	SUM INT PROD UNION INTER
%term	LPILE PILE CPILE RPILE ABOVE
%term	DEFINE TDEFINE NDEFINE DELIM GSIZE GFONT INCLUDE
%right	FROM TO
%left	OVER SQRT
%right	SUP SUB
%right	SIZE FONT ROMAN ITALIC BOLD FAT
%right	UP DOWN BACK FWD
%left	LEFT RIGHT
%right	DOT DOTDOT HAT TILDE BAR UNDER VEC DYAD

%%

stuff	: eqn 	{ putout($1); }
	| error	{ error(!FATAL, "syntax error"); }
	|	{ eqnreg = 0; }
	;

eqn	: box
	| eqn box	{ eqnbox($1, $2, 0); }
	| eqn lineupbox	{ eqnbox($1, $2, 1); }
	| LINEUP	{ lineup(0); }
	;

lineupbox: LINEUP box	{ $$ = $2; lineup(1); }
	;

matrix	: MATRIX	{ $$ = ct; } ;

collist	: column
	| collist column
	;

column	: lcol '{' list '}'	{ column('L', $1); }
	| ccol '{' list '}'	{ column('C', $1); }
	| rcol '{' list '}'	{ column('R', $1); }
	| col '{' list '}'	{ column('-', $1); }
	;

lcol	: LCOL		{ $$ = ct++; } ;
ccol	: CCOL		{ $$ = ct++; } ;
rcol	: RCOL		{ $$ = ct++; } ;
col	: COL		{ $$ = ct++; } ;

sbox	: sup box	%prec SUP	{ $$ = $2; }
	;

tbox	: to box	%prec TO	{ $$ = $2; }
	|		%prec FROM	{ $$ = 0; }
	;

box	: box OVER box			{ boverb($1, $3); }
	| MARK box			{ mark($2); }
	| size box	%prec SIZE	{ size($1, $2); }
	| font box	%prec FONT	{ font($1, $2); }
	| FAT box			{ fatbox($2); }
	| SQRT box			{ eqsqrt($2); }
	| lpile '{' list '}'		{ lpile('L', $1, ct); ct = $1; }
	| cpile '{' list '}'		{ lpile('C', $1, ct); ct = $1; }
	| rpile '{' list '}'		{ lpile('R', $1, ct); ct = $1; }
	| pile '{' list '}'		{ lpile('-', $1, ct); ct = $1; }
	| box sub box sbox %prec SUB	{ shift2($1, $3, $4); }
	| box sub box	   %prec SUB	{ bshiftb($1, $2, $3); }
	| box sup box	   %prec SUP	{ bshiftb($1, $2, $3); }
	| int sub box sbox %prec SUB	{ integral($1, $3, $4); }
	| int sub box	   %prec SUB	{ integral($1, $3, 0); }
	| int sup box	   %prec SUP	{ integral($1, 0, $3); }
	| int				{ integral($1, 0, 0); }
	| left eqn right		{ paren($1, $2, $3); }
	| pbox
	| box from box tbox %prec FROM	{ fromto($1, $3, $4); fromflg=0; }
	| box to box	%prec TO	{ fromto($1, 0, $3); }
	| box diacrit	%prec HAT	{ diacrit($1, $2); }
	| fwd box	%prec UP	{ move(FWD, $1, $2); }
	| up box	%prec UP	{ move(UP, $1, $2); }
	| back box	%prec UP	{ move(BACK, $1, $2); }
	| down box	%prec UP	{ move(DOWN, $1, $2); }
	| matrix '{' collist '}'	{ matrix($1); }
	;

int	: INT	{ setintegral(); }
	;

fwd	: FWD text	{ $$ = atoi((char *) $1); } ;
up	: UP text	{ $$ = atoi((char *) $1); } ;
back	: BACK text	{ $$ = atoi((char *) $1); } ;
down	: DOWN text	{ $$ = atoi((char *) $1); } ;

diacrit	: HAT		{ $$ = HAT; }
	| VEC		{ $$ = VEC; }
	| DYAD		{ $$ = DYAD; }
	| BAR		{ $$ = BAR; }
	| UNDER		{ $$ = UNDER; }	/* under bar */
	| DOT		{ $$ = DOT; }
	| TILDE		{ $$ = TILDE; }
	| DOTDOT	{ $$ = DOTDOT; } /* umlaut = double dot */
	;

from	: FROM	{
		$$=ps;
		ps -= 3;
		fromflg = 1;
		if (dbg)
			printf(".\tfrom: old ps %d, new ps %d, fflg %d\n",
			    $$, ps, fromflg);
	}
	;

to	: TO	{
		$$=ps;
		if (fromflg==0)
			ps -= 3; 
		if(dbg)
			printf(".\tto: old ps %d, new ps %d\n", $$, ps);
	}
	;

left	: LEFT text	{ $$ = ((char *)$2)[0]; }
	| LEFT '{'	{ $$ = '{'; }
	;

right	: RIGHT text	{ $$ = ((char *)$2)[0]; }
	| RIGHT '}'	{ $$ = '}'; }
	|		{ $$ = 0; }
	;

list	: eqn			{ lp[ct++] = $1; }
	| list ABOVE eqn	{ lp[ct++] = $3; }
	;

lpile	: LPILE	{ $$ = ct; } ;
cpile	: CPILE	{ $$ = ct; } ;
pile	: PILE	{ $$ = ct; } ;
rpile	: RPILE	{ $$ = ct; } ;

size	: SIZE text	{ $$ = ps; setsize((char *) $2); }
	;

font	: ROMAN		{ setfont(ROM); }
	| ITALIC	{ setfont(ITAL); }
	| BOLD		{ setfont(BLD); }
	| FONT text	{ setfont(((char *)$2)[0]); }
	;

sub	: SUB	{ shift(SUB); }
	;

sup	: SUP	{ shift(SUP); }
	;

pbox	: '{' eqn '}'	{ $$ = $2; }
	| QTEXT		{ text(QTEXT, (char *) $1); }
	| CONTIG	{ text(CONTIG, (char *) $1); }
	| SPACE		{ text(SPACE, 0); }
	| THIN		{ text(THIN, 0); }
	| TAB		{ text(TAB, 0); }
	| SUM		{ funny(SUM); }
	| PROD		{ funny(PROD); }
	| UNION		{ funny(UNION); }
	| INTER		{ funny(INTER); }	/* intersection */
	;

text	: CONTIG
	| QTEXT
	;

%%

void
diacrit(int p1, int type)
{
	int c, t, effps;

	c = oalloc();
	t = oalloc();
	if (neqn) {
		nrwid(p1, ps, p1);
		printf(".nr 10 %du\n", max(eht[p1]-ebase[p1]-VERT(2),0));
	} else {
		effps = EFFPS(ps);
		nrwid(p1, effps, p1);
		/* vertical shift if high */
		printf(".nr 10 %du\n", VERT(max(eht[p1]-ebase[p1]-6*ps,0)));
		printf(".if \\n(ct>1 .nr 10 \\n(10+\\s%d.25m\\s0\n", effps);
		/* horiz shift if high */
		printf(".nr %d \\s%d.1m\\s0\n", t, effps);
		printf(".if \\n(ct>1 .nr %d \\s%d.15m\\s0\n", t, effps);
	}
	switch(type) {
	case VEC:	/* vec */
		if (!neqn) {
			printf(".ds %d \\v'-.4m'\\s%d\\(->\\s0\\v'.4m'\n",
			    c, max(effps-3, 6));
		}
		break;
	case DYAD:	/* dyad */
		if (neqn)
			printf(".ds %d \\v'-1'_\\v'1'\n", c);
		else
			printf(".ds %d \\v'-.4m'\\s%d\\z\\(<-\\(->\\s0\\v'.4m'\n",
			    c, max(effps-3, 6));
		break;
	case HAT:
		printf(".ds %d ^\n", c);
		break;
	case TILDE:
		printf(".ds %d ~\n", c);
		break;
	case DOT:
		if (neqn)
			printf(".ds %d \\v'-1'.\\v'1'\n", c);
		else
			printf(".ds %d \\s%d\\v'-.67m'.\\v'.67m'\\s0\n",
			    c, effps);
		break;
	case DOTDOT:
		if (neqn)
			printf(".ds %d \\v'-1'..\\v'1'\n", c);
		else
			printf(".ds %d \\s%d\\v'-.67m'..\\v'.67m\\s0'\n",
			    c, effps);
		break;
	case BAR:
		if (neqn)
			printf(".ds %d \\v'-1'\\l'\\n(%du'\\v'1'\n", c, p1);
		else
			printf(".ds %d \\s%d\\v'.18m'\\h'.05m'\\l'\\n(%du-.1m"
			    "\\(rn'\\h'.05m'\\v'-.18m'\\s0\n", c, effps, p1);
		break;
	case UNDER:
		if (neqn)
			printf(".ds %d \\l'\\n(%du'\n", c, p1);
		else {
			printf(".ds %d \\l'\\n(%du\\(ul'\n", c, p1);
			printf(".nr %d 0\n", t);
			printf(".nr 10 0-%d\n", ebase[p1]);
		}
		break;
	}

	nrwid(c, ps, c);
	if (neqn) {
		printf(".as %d \\h'-\\n(%du-\\n(%du/2u'\\v'0-\\n(10u'\\*(%d", 
		    p1, p1, c, c);
		printf("\\v'\\n(10u'\\h'-\\n(%du+\\n(%du/2u'\n", c, p1);
		if (type != UNDER)
			eht[p1] += VERT(1);
		if (dbg)
			printf(".\tdiacrit: %c over S%d, h=%d, b=%d\n",
			    type, p1, eht[p1], ebase[p1]);
	} else {
		if (lfont[p1] != ITAL)
			printf(".nr %d 0\n", t);
		printf(".as %d \\h'-\\n(%du-\\n(%du/2u+\\n(%du'\\v'0-\\n(10u'\\*(%d", 
		    p1, p1, c, t, c);
		printf("\\v'\\n(10u'\\h'-\\n(%du+\\n(%du/2u-\\n(%du'\n", c, p1, t);
		/* BUG - should go to right end of widest */
		if (type != UNDER)
			eht[p1] += VERT( (6*ps*15) / 100);	/* 0.15m */
		if (dbg)
			printf(".\tdiacrit: %c over S%d, lf=%c, rf=%c, h=%d,b=%d\n",
			    type, p1, lfont[p1], rfont[p1], eht[p1], ebase[p1]);
	}

	ofree(c);
	ofree(t);
}

void
eqnbox(int p1, int p2, int lu)
{
	int b, h;
	char *sh;

	yyval = p1;
	b = max(ebase[p1], ebase[p2]);
	eht[yyval] = h = b + max(eht[p1]-ebase[p1], 
		eht[p2]-ebase[p2]);
	ebase[yyval] = b;
	if(dbg)printf(".\te:eb: S%d <- S%d S%d; b=%d, h=%d\n", 
		yyval, p1, p2, b, h);
	if (rfont[p1] == ITAL && lfont[p2] == ROM)
		sh = "\\|";
	else
		sh = "";
	if (lu) {
		printf(".nr %d \\w'\\s%d\\*(%d%s'\n", p1, ps, p1, sh);
		printf(".ds %d \\h'|\\n(97u-\\n(%du'\\*(%d\n", p1, p1, p1);
	}
	printf(".as %d \"%s\\*(%d\n", yyval, sh, p2);
	rfont[p1] = rfont[p2];
	ofree(p2);
}

void
setfont(char ch1)
{
	/* use number '1', '2', '3' for roman, italic, bold */
	yyval = ft;
	if (ch1 == 'r' || ch1 == 'R')
		ft = ROM;
	else if (ch1 == 'i' || ch1 == 'I')
		ft = ITAL;
	else if (ch1 == 'b' || ch1 == 'B')
		ft = BLD;
	else
		ft = ch1;
	printf(".ft %c\n", ft);
	if (dbg) {
		if (neqn)
			printf(".\tsetfont %c\n", ft);
		else
			printf(".\tsetfont %c %c\n", ch1, ft);
	}
}

void
font(int p1, int p2)
{
	/* old font in p1, new in ft */
	yyval = p2;
	lfont[yyval] = rfont[yyval] = ft==ITAL ? ITAL : ROM;
	if(dbg)
		printf(".\tb:fb: S%d <- \\f%c S%d \\f%c b=%d,h=%d,lf=%c,rf=%c\n", 
		    yyval, ft, p2, p1, ebase[yyval], eht[yyval], lfont[yyval], rfont[yyval]);
	printf(".ds %d \\f%c\\*(%d\\f%c\n", yyval, ft, p2, p1);
	ft = p1;
	printf(".ft %c\n", ft);
}

void
fatbox(int p)
{
	int sh;

	yyval = p;
	sh = ps / 4;
	nrwid(p, ps, p);
	printf(".ds %d \\*(%d\\h'-\\n(%du+%du'\\*(%d\n", p, p, p, sh, p);
	if (dbg)
		printf(".\tfat %d, sh=%d\n", p, sh);
}

void
fromto(int p1, int p2, int p3)
{
	int b, h1, b1, pss;

	yyval = oalloc();
	lfont[yyval] = rfont[yyval] = 0;
	h1 = eht[yyval] = eht[p1];
	b1 = ebase[p1];
	b = 0;
	pss = EFFPS(ps);
	ps += 3;
	nrwid(p1, ps, p1);
	printf(".nr %d \\n(%d\n", yyval, p1);
	if (p2 > 0) {
		nrwid(p2, pss, p2);
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n",
		    p2, yyval, yyval, p2);
		eht[yyval] += eht[p2];
		b = eht[p2];
	}
	if (p3 > 0) {
		nrwid(p3, pss, p3);
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n",
		    p3, yyval, yyval, p3);
		eht[yyval] += eht[p3];
	}
	printf(".ds %d ", yyval);	/* bottom of middle box */
	if (p2 > 0) {
		printf("\\v'%du'\\h'\\n(%du-\\n(%du/2u'\\s%d\\*(%d\\s%d", 
		    eht[p2]-ebase[p2]+b1, yyval, p2, pss, p2, EFFPS(ps));
		printf("\\h'-\\n(%du-\\n(%du/2u'\\v'%du'\\\n", 
		    yyval, p2, -(eht[p2]-ebase[p2]+b1));
	}
	if (neqn)
		printf("\\h'\\n(%du-\\n(%du/2u'\\*(%d\\h'\\n(%du-\\n(%du+2u/2u'\\\n", 
		    yyval, p1, p1, yyval, p1);
	else
		printf("\\h'\\n(%du-\\n(%du/2u'\\*(%d\\h'\\n(%du-\\n(%du/2u'\\\n", 
		    yyval, p1, p1, yyval, p1);
	if (p3 > 0) {
		printf("\\v'%du'\\h'-\\n(%du-\\n(%du/2u'\\s%d\\*(%d\\s%d\\h'\\n(%du-\\n(%du/2u'\\v'%du'\\\n", 
		    -(h1-b1+ebase[p3]), yyval, p3, pss, p3, EFFPS(ps),
		    yyval, p3, (h1-b1+ebase[p3]));
	}
	printf("\n");
	ebase[yyval] = b + b1;
	if (dbg)
		printf(".\tfrom to: S%d <- %d f %d t %d; h=%d b=%d\n", 
		    yyval, p1, p2, p3, eht[yyval], ebase[yyval]);
	ofree(p1);
	if (p2 > 0)
		ofree(p2);
	if (p3 > 0)
		ofree(p3);
}

void
funny(int n)
{
	char *f;

	yyval = oalloc();
	switch(n) {
	case SUM:
		f = "\\(*S";
		break;
	case UNION:
		f = "\\(cu";
		break;
	case INTER:	/* intersection */
		f = "\\(ca";
		break;
	case PROD:
		f = "\\(*P"; break;
	default:
		error(FATAL, "funny type %d in funny", n);
	}
	if (neqn) {
		printf(".ds %d %s\n", yyval, f);
		eht[yyval] = VERT(2);
		ebase[yyval] = 0;
	} else {
		printf(".ds %d \\s%d\\v'.3m'\\s+5%s\\s-5\\v'-.3m'\\s%d\n",
		    yyval, ps, f, ps);
		eht[yyval] = VERT( (ps+5)*6 -(ps*6*2)/10 );
		ebase[yyval] = VERT( (ps*6*3)/10 );
	}
	if(dbg)
		printf(".\tfunny: S%d <- %s; h=%d b=%d\n", 
		    yyval, f, eht[yyval], ebase[yyval]);
	lfont[yyval] = rfont[yyval] = ROM;
}

void
integral(int p, int p1, int p2)
{
	if (!neqn) {
		if (p1 != 0)
			printf(".ds %d \\h'-0.4m'\\v'0.4m'\\*(%d\\v'-0.4m'\n",
			    p1, p1);
		if (p2 != 0)
			printf(".ds %d \\v'-0.3m'\\*(%d\\v'0.3m'\n", p2, p2);
	}
	if (p1 != 0 && p2 != 0)
		shift2(p, p1, p2);
	else if (p1 != 0)
		bshiftb(p, SUB, p1);
	else if (p2 != 0)
		bshiftb(p, SUP, p2);
	if(dbg)
		printf(".\tintegral: S%d; h=%d b=%d\n", 
		    p, eht[p], ebase[p]);
	lfont[p] = ROM;
}

void
setintegral(void)
{
	char *f;

	yyval = oalloc();
	f = "\\(is";
	if (neqn) {
		printf(".ds %d %s\n", yyval, f);
		eht[yyval] = VERT(2);
		ebase[yyval] = 0;
	} else {
		printf(".ds %d \\s%d\\v'.1m'\\s+4%s\\s-4\\v'-.1m'\\s%d\n", 
		    yyval, ps, f, ps);
		eht[yyval] = VERT( (((ps+4)*12)/10)*6 );
		ebase[yyval] = VERT( (ps*6*3)/10 );
	}
	lfont[yyval] = rfont[yyval] = ROM;
}

void
mark(int p1)
{
	markline = 1;
	printf(".ds %d \\k(97\\*(%d\n", p1, p1);
	yyval = p1;
	if (dbg)
		printf(".\tmark %d\n", p1);
}

void
lineup(int p1)
{
	markline = 1;
	if (p1 == 0) {
		yyval = oalloc();
		printf(".ds %d \\h'|\\n(97u'\n", yyval);
	}
	if(dbg)
		printf(".\tlineup %d\n", p1);
}

void
column(int type, int p1)
{
	int i;

	lp[p1] = ct - p1 - 1;
	if (dbg) {
		printf(".\t%d column of", type);
		for (i = p1 + 1; i < ct; i++)
			printf(" S%d", lp[i]);
		printf(", rows=%d\n",lp[p1]);
	}
	lp[ct++] = type;
}

void
matrix(int p1)
{
	int nrow, ncol, i, j, k, hb, b, val[100];
	char *space;

	space = "\\ \\ ";
	nrow = lp[p1];	/* disaster if rows inconsistent */
	ncol = 0;
	for (i = p1; i < ct; i += lp[i] + 2) {
		ncol++;
		if (dbg)
			printf(".\tcolct=%d\n",lp[i]);
	}
	for (k = 1; k <= nrow; k++) {
		hb = b = 0;
		j = p1 + k;
		for (i = 0; i < ncol; i++) {
			hb = max(hb, eht[lp[j]]-ebase[lp[j]]);
			b = max(b, ebase[lp[j]]);
			j += nrow + 2;
		}
		if (dbg)
			printf(".\trow %d: b=%d, hb=%d\n", k, b, hb);
		j = p1 + k;
		for (i = 0; i < ncol; i++) {
			ebase[lp[j]] = b;
			eht[lp[j]] = b + hb;
			j += nrow + 2;
		}
	}
	j = p1;
	for (i = 0; i < ncol; i++) {
		lpile(lp[j+lp[j]+1], j+1, j+lp[j]+1);
		val[i] = yyval;
		j += nrow + 2;
	}
	yyval = oalloc();
	eht[yyval] = eht[val[0]];
	ebase[yyval] = ebase[val[0]];
	lfont[yyval] = rfont[yyval] = 0;
	if(dbg)
		printf(".\tmatrix S%d: r=%d, c=%d, h=%d, b=%d\n",
		    yyval,nrow,ncol,eht[yyval],ebase[yyval]);
	printf(".ds %d \"", yyval);
	for (i = 0; i < ncol; i++)  {
		printf("\\*(%d%s", val[i], i==ncol-1 ? "" : space);
		ofree(val[i]);
	}
	printf("\n");
	ct = p1;
}

void
move(int dir, int amt, int p)
{
	int a;

	yyval = p;
	if (neqn)
		/* nearest number of half-lines */
		a = VERT((amt + 49) / 50);
	else
		a = VERT((EFFPS(ps) * 6 * amt) / 100);
	printf(".ds %d ", yyval);
	if (dir == FWD || dir == BACK)	/* fwd, back */
		printf("\\h'%s%du'\\*(%d\n", (dir==BACK) ? "-" : "", a, p);
	else if (dir == UP)
		printf("\\v'-%du'\\*(%d\\v'%du'\n", a, p, a);
	else if (dir == DOWN)
		printf("\\v'%du'\\*(%d\\v'-%du'\n", a, p, a);
	if (dbg)
		printf(".\tmove %d dir %d amt %d; h=%d b=%d\n", 
		    p, dir, a, eht[yyval], ebase[yyval]);
}

void
boverb(int p1, int p2)
{
	int h, b, treg, d;

	treg = oalloc();
	yyval = p1;
	if (neqn) {
		d = VERT(1);
		h = eht[p1] + eht[p2];
	} else {
		d = VERT((ps*6*3) / 10);	/* 0.3m */
		h = eht[p1] + eht[p2] + d;
	}
	b = eht[p2] - d;
	if (dbg)
		printf(".\tb:bob: S%d <- S%d over S%d; b=%d, h=%d\n", 
		    yyval, p1, p2, b, h);
	nrwid(p1, ps, p1);
	nrwid(p2, ps, p2);
	printf(".nr %d \\n(%d\n", treg, p1);
	printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", p2, treg, treg, p2);
	if (!neqn)
		printf(".nr %d \\n(%d+\\s%d.5m\\s0\n", treg, treg, EFFPS(ps));
	printf(".ds %d \\v'%du'\\h'\\n(%du-\\n(%du/2u'\\*(%d\\\n", 
	    yyval, eht[p2]-ebase[p2]-d, treg, p2, p2);
	if (neqn) {
		printf("\\h'-\\n(%du-\\n(%du/2u'\\v'%du'\\*(%d\\\n", 
		    p2, p1, -eht[p2]+ebase[p2]-ebase[p1], p1);
		printf("\\h'-\\n(%du-\\n(%du-2u/2u'\\v'%du'\\l'\\n(%du'\\v'%du'\n", 
		    treg, p1, ebase[p1], treg, d);
	} else {
		printf("\\h'-\\n(%du-\\n(%du/2u'\\v'%du'\\*(%d\\\n", 
		    p2, p1, -(eht[p2]-ebase[p2]+d+ebase[p1]), p1);
		printf("\\h'-\\n(%du-\\n(%du/2u+.1m'\\v'%du'\\l'\\n(%du-.2m"
		    "'\\h'.1m'\\v'%du'\n", treg, p1, ebase[p1]+d, treg, d);
	}
	ebase[yyval] = b;
	eht[yyval] = h;
	lfont[yyval] = rfont[yyval] = 0;
	ofree(p2);
	ofree(treg);
}

void
paren(int leftc, int p1, int rightc)
{
	int n, m, h1, j, b1, v;
	h1 = eht[p1]; b1 = ebase[p1];
	yyval = p1;
	if (neqn)
		n = max(b1+VERT(1), h1-b1-VERT(1)) / VERT(1);
	else {
		lfont[yyval] = rfont[yyval] = 0;
		n = (h1+(6*EFFPS(ps)-1))/(6*EFFPS(ps));
	}
	if (n < 2)
		n = 1;
	m = n - 2;
	if (leftc == '{' || rightc == '}') {
		n = n % 2 ? n : ++n;
		if (n < 3)
			n = 3;
		m = n - 3;
	}
	if (neqn) {
		eht[yyval] = VERT(2 * n);
		ebase[yyval] = (n)/2 * VERT(2);
		if (n % 2 == 0)
			ebase[yyval] -= VERT(1);
		v = b1 - h1/2 + VERT(1);
	} else {
		eht[yyval] = VERT(6 * ps * n);
		ebase[yyval] = b1 + (eht[yyval]-h1)/2;
		v = b1 - h1/2 + VERT( (ps*6*4)/10 );
	}
	printf(".ds %d \\|\\v'%du'", yyval, v);
	switch( leftc ) {
	case 'n':	/* nothing */
	case '\0':
		break;
	case 'f':	/* floor */
		if (n <= 1)
			printf("\\(lf");
		else
			brack(m, "\\(bv", "\\(bv", "\\(lf");
		break;
	case 'c':	/* ceiling */
		if (n <= 1)
			printf("\\(lc");
		else
			brack(m, "\\(lc", "\\(bv", "\\(bv");
		break;
	case '{':
		printf("\\b'\\(lt");
		for(j = 0; j < m; j += 2) printf("\\(bv");
		printf("\\(lk");
		for(j = 0; j < m; j += 2) printf("\\(bv");
		printf("\\(lb'");
		break;
	case '(':
		brack(m, "\\(lt", "\\(bv", "\\(lb");
		break;
	case '[':
		brack(m, "\\(lc", "\\(bv", "\\(lf");
		break;
	case '|':
		brack(m, "|", "|", "|");
		break;
	default:
		brack(m, (char *) &leftc, (char *) &leftc, (char *) &leftc);
		break;
	}
	printf("\\v'%du'\\*(%d", -v, p1);
	if (rightc) {
		printf("\\|\\v'%du'", v);
		switch (rightc) {
		case 'f':	/* floor */
			if (n <= 1)
				printf("\\(rf");
			else
				brack(m, "\\(bv", "\\(bv", "\\(rf");
			break;
		case 'c':	/* ceiling */
			if (n <= 1)
				printf("\\(rc");
			else
				brack(m, "\\(rc", "\\(bv", "\\(bv");
			break;
		case '}':
			printf("\\b'\\(rt");
			for(j = 0; j< m; j += 2)printf("\\(bv");
			printf("\\(rk");
			for(j = 0; j< m; j += 2) printf("\\(bv");
			printf("\\(rb'");
			break;
		case ']':
			brack(m, "\\(rc", "\\(bv", "\\(rf");
			break;
		case ')':
			brack(m, "\\(rt", "\\(bv", "\\(rb");
			break;
		case '|':
			brack(m, "|", "|", "|");
			break;
		default:
			brack(m, (char *)&rightc, (char *)&rightc, (char *)&rightc);
			break;
		}
		printf("\\v'%du'", -v);
	}
	printf("\n");
	if (dbg)
		printf(".\tcurly: h=%d b=%d n=%d v=%d l=%c, r=%c\n", 
		    eht[yyval], ebase[yyval], n, v, leftc, rightc);
}

void
brack(int m, char *t, char *c, char *b)
{
	int j;
	printf("\\b'%s", t);
	for (j = 0; j < m; j++)
		printf("%s", c);
	printf("%s'", b);
}

void
lpile(int type, int p1, int p2)
{
	int bi, hi, i, gap, h, b, nlist, nlist2, mid;

	yyval = oalloc();
	if (neqn)
		gap = VERT(1);
	else
		gap = VERT( (ps*6*4)/10 ); /* 4/10 m between blocks */

	if (type == '-')
		gap = 0;
	nlist = p2 - p1;
	nlist2 = (nlist+1)/2;
	mid = p1 + nlist2 -1;
	h = 0;
	for (i = p1; i < p2; i++)
		h += eht[lp[i]];
	eht[yyval] = h + (nlist-1)*gap;
	b = 0;
	for (i = p2 - 1; i > mid; i--)
		b += eht[lp[i]] + gap;
	ebase[yyval] = (nlist%2) ? b + ebase[lp[mid]]
	    : neqn? b - VERT(1) - gap : b - VERT((ps * 6 * 5) / 10) - gap;
	if (dbg) {
		printf(".\tS%d <- %c pile of:", yyval, type);
		for (i = p1; i < p2; i++)
			printf(" S%d", lp[i]);
		printf(";h=%d b=%d\n", eht[yyval], ebase[yyval]);
	}
	nrwid(lp[p1], ps, lp[p1]);
	printf(".nr %d \\n(%d\n", yyval, lp[p1]);
	for (i = p1 + 1; i < p2; i++) {
		nrwid(lp[i], ps, lp[i]);
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", 
		    lp[i], yyval, yyval, lp[i]);
	}
	printf(".ds %d \\v'%du'\\h'%du*\\n(%du'\\\n", yyval, ebase[yyval], 
	    type=='R' ? 1 : 0, yyval);
	for (i = p2 - 1; i >= p1; i--) {
		hi = eht[lp[i]]; 
		bi = ebase[lp[i]];
		switch(type) {
		case 'L':
			printf("\\v'%du'\\*(%d\\h'-\\n(%du'\\v'0-%du'\\\n", 
			    -bi, lp[i], lp[i], hi-bi+gap);
			continue;
		case 'R':
			printf("\\v'%du'\\h'-\\n(%du'\\*(%d\\v'0-%du'\\\n", 
			    -bi, lp[i], lp[i], hi-bi+gap);
			continue;
		case 'C':
		case '-':
			printf("\\v'%du'\\h'\\n(%du-\\n(%du/2u'\\*(%d", 
			    -bi, yyval, lp[i], lp[i]);
			printf("\\h'-\\n(%du-\\n(%du/2u'\\v'0-%du'\\\n", 
			    yyval, lp[i], hi-bi+gap);
			continue;
		}
	}
	printf("\\v'%du'\\h'%du*\\n(%du'\n", eht[yyval]-ebase[yyval]+gap, 
	    type != 'R' ? 1 : 0, yyval);
	for (i = p1; i < p2; i++)
		ofree(lp[i]);
	lfont[yyval] = rfont[yyval] = 0;
}

void
bshiftb(int p1, int dir, int p2)
{
	int shval, d1, h1, b1, h2, b2, diffps, effps, effps2;
	char *sh1, *sh2;

	yyval = p1;
	h1 = eht[p1];
	b1 = ebase[p1];
	h2 = eht[p2];
	b2 = ebase[p2];
	if (!neqn) {
		effps = EFFPS(ps);
		effps2 = EFFPS(ps+deltaps);
		diffps = deltaps;
		sh1 = sh2 = "";
	}
	if (dir == SUB) {	/* subscript */
		/* top 1/2m above bottom of main box */
		d1 = neqn? VERT(1) : VERT((effps2 * 6) / 2);
		shval = - d1 + h2 - b2;
		if( d1+b1 > h2 ) /* move little sub down */
			shval = b1-b2;
		ebase[yyval] = b1 + max(0, h2-b1-d1);
		eht[yyval] = h1 + max(0, h2-b1-d1);
		if (!neqn) {
			if (rfont[p1] == ITAL && lfont[p2] == ROM)
				sh1 = "\\|";
			if (rfont[p2] == ITAL)
				sh2 = "\\|";
		}
	} else {	/* superscript */
		/* 4/10 up main box */
		d1 = neqn? VERT(1) : VERT((effps * 6 * 2) / 10);
		ebase[yyval] = b1;
		if (neqn) {
			shval = -VERT(1) - b2;
			if (VERT(1) + h2 < h1 - b1)
				/* raise little super */
				shval = -(h1-b1) + h2-b2 - d1;
		} else {
			shval = -VERT((4 * (h1 - b1)) / 10) - b2;
			if (VERT(4 * (h1 - b1) / 10) + h2 < h1 - b1 )
				/* raise little super */
				shval = -(h1-b1) + h2-b2 - d1;
		}
		if (neqn)
			eht[yyval] = h1 + max(0, h2 - VERT(1));
		else {
			eht[yyval] = h1 + max(0, h2-VERT((6*(h1-b1))/10));
			if (rfont[p1] == ITAL)
				sh1 = "\\|";
			if (rfont[p2] == ITAL)
				sh2 = "\\|";
		}
	}
	if(dbg)
		printf(".\tb:b shift b: S%d <- S%d vert %d S%d vert %d; b=%d, h=%d\n", 
		    yyval, p1, shval, p2, -shval, ebase[yyval], eht[yyval]);
	if (neqn)
		printf(".as %d \\v'%du'\\*(%d\\v'%du'\n",
		    yyval, shval, p2, -shval);
	else {
		printf(".as %d \\v'%du'\\s-%d%s\\*(%d\\s+%d%s\\v'%du'\n", 
		    yyval, shval, diffps, sh1, p2, diffps, sh2, -shval);
		ps += deltaps;
		if (rfont[p2] == ITAL)
			rfont[p1] = 0;
		else
			rfont[p1] = rfont[p2];
	}
	ofree(p2);
}

void
shift(int p1)
{
	ps -= deltaps;
	yyval = p1;
	if (dbg)
		printf(".\tshift: %d;ps=%d\n", yyval, ps);
}

void
shift2(int p1, int p2, int p3)
{
	int effps, effps2, h1, h2, h3, b1, b2, b3, subsh, d1, d2, supsh, treg;

	treg = oalloc();
	yyval = p1;
	if (dbg)
		printf(".\tshift2 s%d <- %d %d %d\n", yyval, p1, p2, p3);
	effps = EFFPS(ps+deltaps);

	if (!neqn) {
		eht[p3] = h3 = VERT( (eht[p3] * effps) / EFFPS(ps) );
		ps += deltaps;
		effps2 = EFFPS(ps+deltaps);
	}
	h1 = eht[p1]; b1 = ebase[p1];
	h2 = eht[p2]; b2 = ebase[p2];
	if (neqn) {
		h3 = eht[p3]; b3 = ebase[p3];
		d1 = VERT(1);
	} else {
		b3 = ebase[p3];
		d1 = VERT((effps2 * 6) / 2);
	}
	subsh = -d1+h2-b2;
	if (d1 + b1 > h2)	/* move little sub down */
		subsh = b1 - b2;
	if (neqn) {
		supsh = - VERT(1) - b3;
		d2 = VERT(1);
		if (VERT(1) + h3 < h1 - b1)
			supsh = -(h1 - b1) + (h3 - b3) - d2;
		eht[yyval] = h1 + max(0, h3-VERT(1)) + max(0, h2-b1-d1);
	} else {
		supsh = -VERT((4 * (h1 - b1)) / 10) - b3;
		d2 = VERT((effps * 6 * 2) / 10);
		if (VERT(4 * (h1 - b1) / 10) + h3 < h1 - b1)
			supsh = -(h1 - b1) + (h3 - b3) - d2;
		eht[yyval] = h1 + max(0, h3-VERT( (6*(h1-b1))/10 )) +
		    max(0, h2-b1-d1);
	}
	ebase[yyval] = b1 + max(0, h2 - b1 - d1);
	if (!neqn) {
		if (rfont[p1] == ITAL && lfont[p2] == ROM)
			printf(".ds %d \\|\\*(%d\n", p2, p2);
		if (rfont[p2] == ITAL)
			printf(".as %d \\|\n", p2);
	}
	nrwid(p2, effps, p2);
	if (!neqn) {
		if (rfont[p1] == ITAL && lfont[p3] == ROM)
			printf(".ds %d \\|\\|\\*(%d\n", p3, p3);
		else
			printf(".ds %d \\|\\*(%d\n", p3, p3);
	}

	nrwid(p3, effps, p3);
	printf(".nr %d \\n(%d\n", treg, p3);
	printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", p2, treg, treg, p2);
	if (neqn) {
		printf(".as %d \\v'%du'\\*(%d\\h'-\\n(%du'\\v'%du'\\\n", 
		    p1, subsh, p2, p2, -subsh+supsh);
		printf("\\*(%d\\h'-\\n(%du+\\n(%du'\\v'%du'\n", 
		    p3, p3, treg, -supsh);
	} else {
		printf(".as %d \\v'%du'\\s%d\\*(%d\\h'-\\n(%du'\\v'%du'\\\n", 
		    p1, subsh, effps, p2, p2, -subsh+supsh);
		printf("\\s%d\\*(%d\\h'-\\n(%du+\\n(%du'\\s%d\\v'%du'\n", 
		    effps, p3, p3, treg, effps2, -supsh);
	}
	ps += deltaps;
	if (!neqn)
		if (rfont[p2] == ITAL)
			rfont[yyval] = 0;	/* lie */
	ofree(p2);
	ofree(p3);
	ofree(treg);
}

void
setsize(char *p)	/* set size as found in p */
{
	if (*p == '+')
		ps += atoi(p+1);
	else if (*p == '-')
		ps -= atoi(p+1);
	else
		ps = atoi(p);
	if (dbg)
		printf(".\tsetsize %s; ps = %d\n", p, ps);
}

void
size(int p1, int p2)
{
	/* old size in p1, new in ps */
	int effps, effp1;

	yyval = p2;
	if (dbg)
		printf(".\tb:sb: S%d <- \\s%d S%d \\s%d; b=%d, h=%d\n", 
		    yyval, ps, p2, p1, ebase[yyval], eht[yyval]);
	effps = EFFPS(ps);
	effp1 = EFFPS(p1);
	printf(".ds %d \\s%d\\*(%d\\s%d\n", yyval, effps, p2, effp1);
	ps = p1;
}

void
eqsqrt(int p2)
{
	int nps;

	if (!neqn)
		nps = EFFPS(((eht[p2]*9)/10+5)/6);
	yyval = p2;
	if (!neqn) {
		eht[yyval] = VERT( (nps*6*12)/10 );
		if(dbg)
			printf(".\tsqrt: S%d <- S%d;b=%d, h=%d\n", 
			    yyval, p2, ebase[yyval], eht[yyval]);
		if (rfont[yyval] == ITAL)
			printf(".as %d \\|\n", yyval);
	}
	nrwid(p2, ps, p2);
	if (neqn) {
		printf(".ds %d \\v'%du'\\e\\L'%du'\\l'\\n(%du'",
		    p2, ebase[p2], -eht[p2], p2);
		printf("\\v'%du'\\h'-\\n(%du'\\*(%d\n",
		    eht[p2]-ebase[p2], p2, p2);
		eht[p2] += VERT(1);
		if(dbg)
			printf(".\tsqrt: S%d <- S%d;b=%d, h=%d\n", 
			    p2, p2, ebase[p2], eht[p2]);
	} else {
		printf(".ds %d \\v'%du'\\s%d\\v'-.2m'\\(sr\\l'\\n(%du\\(rn"
		    "'\\v'.2m'\\s%d", yyval, ebase[p2], nps, p2, ps);
		printf("\\v'%du'\\h'-\\n(%du'\\*(%d\n", -ebase[p2], p2, p2);
		lfont[yyval] = ROM;
	}
}
