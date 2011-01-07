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
static char sccsid[] = "@(#)tablpr.c	4.2 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)n10.c	4.6 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD: n10.c,v 1.1 2011/01/05 23:31:20 mickey Exp $";
#endif
#endif /* not lint */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>
#include <sgtty.h>

#include "tdef.h"

/*
nroff10.c

Device interfaces
*/

int dtab;
int bdmode;
int plotmode;

/*
 * LPR or CRT 10 Pitch
 * nroff driving table
 * no reverse or half line feeds
 * by UCB Computer Center
 */
struct tw tascii = {
/*bset*/	0,
/*breset*/	0,
/*Hor*/		INCH/10,
/*Vert*/	INCH/6,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"",
/*twrest*/	"",
/*twnl*/	"\n",
/*hlr*/		"",
/*hlf*/		"",
/*flr*/		"",
/*bdon*/	"",
/*bdoff*/	"",
/*ploton*/	"",
/*plotoff*/	"",
/*up*/		"",
/*down*/	"",
/*right*/	"",
/*left*/	"",
/*codetab*/
{
"\001 ",	/*space*/
"\001!",	/*!*/
"\001\"",	/*"*/
"\001#",	/*#*/
"\001$",	/*$*/
"\001%",	/*%*/
"\001&",	/*&*/
"\001'",	/*' close*/
"\001(",	/*(*/
"\001)",	/*)*/
"\001*",	/***/
"\001+",	/*+*/
"\001,",	/*,*/
"\001-",	/*-*/
"\001.",	/*.*/
"\001/",	/*/*/
"\2010",	/*0*/
"\2011",	/*1*/
"\2012",	/*2*/
"\2013",	/*3*/
"\2014",	/*4*/
"\2015",	/*5*/
"\2016",	/*6*/
"\2017",	/*7*/
"\2018",	/*8*/
"\2019",	/*9*/
"\001:",	/*:*/
"\001;",	/*;*/
"\001<",	/*<*/
"\001=",	/*=*/
"\001>",	/*>*/
"\001?",	/*?*/
"\001@",	/*@*/
"\201A",	/*A*/
"\201B",	/*B*/
"\201C",	/*C*/
"\201D",	/*D*/
"\201E",	/*E*/
"\201F",	/*F*/
"\201G",	/*G*/
"\201H",	/*H*/
"\201I",	/*I*/
"\201J",	/*J*/
"\201K",	/*K*/
"\201L",	/*L*/
"\201M",	/*M*/
"\201N",	/*N*/
"\201O",	/*O*/
"\201P",	/*P*/
"\201Q",	/*Q*/
"\201R",	/*R*/
"\201S",	/*S*/
"\201T",	/*T*/
"\201U",	/*U*/
"\201V",	/*V*/
"\201W",	/*W*/
"\201X",	/*X*/
"\201Y",	/*Y*/
"\201Z",	/*Z*/
"\001[",	/*[*/
"\001\\",	/*\*/
"\001]",	/*]*/
"\001^",	/*^*/
"\001_",	/*_*/
"\001`",	/*` open*/
"\201a",	/*a*/
"\201b",	/*b*/
"\201c",	/*c*/
"\201d",	/*d*/
"\201e",	/*e*/
"\201f",	/*f*/
"\201g",	/*g*/
"\201h",	/*h*/
"\201i",	/*i*/
"\201j",	/*j*/
"\201k",	/*k*/
"\201l",	/*l*/
"\201m",	/*m*/
"\201n",	/*n*/
"\201o",	/*o*/
"\201p",	/*p*/
"\201q",	/*q*/
"\201r",	/*r*/
"\201s",	/*s*/
"\201t",	/*t*/
"\201u",	/*u*/
"\201v",	/*v*/
"\201w",	/*w*/
"\201x",	/*x*/
"\201y",	/*y*/
"\201z",	/*z*/
"\001{",	/*{*/
"\001|",	/*|*/
"\001}",	/*}*/
"\001~",	/*~*/
"\000",		/*nar sp*/
"\001-",	/*hyphen*/
"\001o\b+",	/*bullet*/
"\002[]",	/*square*/
"\001-",	/*3/4 em*/
"\001_",	/*rule*/
"\0031/4",	/*1/4*/
"\0031/2",	/*1/2*/
"\0033/4",	/*3/4*/
"\001-",	/*minus*/
"\202fi",	/*fi*/
"\202fl",	/*fl*/
"\202ff",	/*ff*/
"\203ffi",	/*ffi*/
"\203ffl",	/*ffl*/
"\000\0",	/*degree*/
"\001|\b-",	/*dagger*/
"\001s\bS",	/*section*/
"\001'",	/*foot mark*/
"\001'",	/*acute accent*/
"\001`",	/*grave accent*/
"\001_",	/*underrule*/
"\001/",	/*slash (longer)*/
"\000",		/*half narrow space*/
"\001 ",	/*unpaddable space*/
"\201o\b(", 	/*alpha*/
"\2018\b|", 	/*beta*/
"\201>\b/", 	/*gamma*/
"\201d\b`", 	/*delta*/
"\201C\b-", 	/*epsilon*/
"\000\0", 	/*zeta*/
"\201n",	/*eta*/
"\201o\b-", 	/*theta*/
"\201i",	/*iota*/
"\201k",	/*kappa*/
"\201,\b\\", 	/*lambda*/
"\201u",	/*mu*/
"\201v",	/*nu*/
"\000\0", 	/*xi*/
"\201o",	/*omicron*/
"\202i\b~i\b~",	/*pi*/
"\201p",	/*rho*/
"\201o\b~", 	/*sigma*/
"\201i\b~", 	/*tau*/
"\201u",	/*upsilon*/
"\201o\b|", 	/*phi*/
"\201x",	/*chi*/
"\201u\b|", 	/*psi*/
"\201w", 	/*omega*/
"\201I\b~", 	/*Gamma*/
"\202/\b_\\\b_", /*Delta*/
"\201O\b-",	/*Theta*/
"\202/\\",	/*Lambda*/
"\201=\b_",	/*Xi*/
"\202TT",	/*Pi*/
"\201>\b_\b~", 	/*Sigma*/
"\000",		/**/
"\201Y",	/*Upsilon*/
"\201O\b|",	/*Phi*/
"\201U\b|",	/*Psi*/
"\201O\b_",	/*Omega*/
"\001v\b/",	/*square root*/
"\000\0",	/*terminal sigma*/
"\001~",	/*root en*/
"\001>\b_",	/*>=*/
"\001<\b_",	/*<=*/
"\001=\b_",	/*identically equal*/
"\001-",	/*equation minus*/
"\001~\b_",	/*approx =*/
"\001~",	/*approximates*/
"\001=\b/",	/*not equal*/
"\002->",	/*right arrow*/
"\002<-",	/*left arrow*/
"\001|\b^",	/*up arrow*/
"\001|\bv",	/*down arrow*/
"\001=",	/*equation equal*/
"\001x",	/*multiply*/
"\001:\b-",	/*divide*/
"\001+\b_",	/*plus-minus*/
"\002(\b~)\b~",	/*cup (union)*/
"\002(\b_)\b_",	/*cap (intersection)*/
"\002(=",	/*subset of*/
"\002=)",	/*superset of*/
"\002(=\b_",	/*improper subset*/
"\002=\b_)",	/*improper superset*/
"\002oo",	/*infinity*/
"\001o\b`",	/*partial derivative*/
"\002\\\b~/\b~", /*gradient*/
"\000\0",	/*not*/
"\000\0",	/*integral sign*/
"\002oc",	/*proportional to*/
"\001O\b/",	/*empty set*/
"\001<\b-",	/*member of*/
"\001+",	/*equation plus*/
"\003(R)",	/*registered*/
"\003(C)",	/*copyright*/
"\001|",	/*box rule */
"\001c\b/",	/*cent sign*/
"\001|\b=",	/*dbl dagger*/
"\002=>",	/*right hand*/
"\002<=",	/*left hand*/
"\001*",	/*math * */
"\000\0",	/*bell system sign*/
"\001|",	/*or (was star)*/
"\001O",	/*circle*/
"\001|",	/*left top of big brace*/
"\001|",	/*left bot of big brace*/
"\001|",	/*right top of big brace*/
"\001|",	/*right bot of big brace*/
"\001|",	/*left center of big brace*/
"\001|",	/*right center of big brace*/
"\001|",	/*bold vertical*/
"\001|",	/*left floor (lb of big bracket)*/
"\001|",	/*right floor (rb of big bracket)*/
"\001|",	/*left ceiling (lt of big bracket)*/
"\001|"		/*right ceiling (rt of big bracket)*/
}
};

void
ptinit(void)
{
	int i;

	if (strcmp(&termtab[tti], "ascii")) {
		char *q, **p;
		int j, x[8];

		if (((i = open(termtab, O_RDONLY)) < 0))
			err(1, "open: %s", termtab);
		if (read(i, x, 8 * sizeof(int)) != 8 * sizeof(int))
			err(1, "read: %s", termtab);
		/* Calc size of table, not counting zzz */
		j = ((int) &t.zzz - (int) &t.bset);
		if (read(i, &t.bset, j) != j)
			err(1, "read %d: %s", j, termtab);
		x[2] -= j + sizeof(int);
		q = setbrk(x[2]);
		lseek(i, (long)t.twinit + 8 * sizeof(int), SEEK_SET);
		if (read(i, q, x[2]) != x[2])
			err(1, "read %d: %s", x[2], termtab);
		close(i);
		j = q - t.twinit;
		for (p = &t.twinit; p < &t.zzz; p++) {
			if (*p)
				*p += j;
			else
				*p = "";
		}
	} else
		t = tascii;

	sps = EM;
	ics = EM*2;
	dtab = 8 * t.Em;
	for (i = 0; i < 16; i++)
		tabtab[i] = dtab * (i + 1);
	if (eqflg)
		t.Adj = t.Hor;
}

void
twdone(void)
{
	obufp = obuf;
	oputs(t.twrest);
	flusho();
	if(pipeflg){
		close(ptid);
		wait(&waitf);
	}
	if(ttysave != -1) {
		ttys.sg_flags = ttysave;
		stty(1, &ttys);
	}
}

void
ptout(int i)
{
	*olinep++ = i;
	if (olinep >= &oline[LNSIZE])
		olinep--;
	if ((i&CMASK) != '\n')
		return;
	olinep--;
	lead += dip->blss + lss - t.Newline;
	dip->blss = 0;
	esct = esc = 0;
	if (olinep>oline) {
		move();
		ptout1();
		oputs(t.twnl);
	} else {
		lead += t.Newline;
		move();
	}
	lead += dip->alss;
	dip->alss = 0;
	olinep = oline;
}

void
ptout1(void)
{
	int *q, w, i, j, k, phyw;
	char *codep;

	for (q = oline; q < olinep; q++) {
	if ((i = *q) & MOT) {
		j = i & ~MOTV;
		if (i & NMOT)
			j = -j;
		if (i & VMOT)
			lead += j;
		else
			esc += j;
		continue;
	}
	if ((k = (i & CMASK)) <= 040) {
		switch(k){
		case ' ':	/* space */
			esc += t.Char;
			break;
		}
		continue;
	}
	codep = t.codetab[k-32];
	w = t.Char * (*codep++ & 0177);
	phyw = w;
	if (i&ZBIT)
		w = 0;
	if (*codep && (esc || lead))
		move();
	esct += w;
	if (i&074000)
		xfont = (i>>9) & 03;
	if (*t.bdon & 0377) {
		if (!bdmode && (xfont == 2)){
			oputs(t.bdon);
			bdmode++;
		}
		if(bdmode && (xfont != 2)){
			oputs(t.bdoff);
			bdmode = 0;
		}
	}

	if(xfont == ulfont){
		for (k = w / t.Char; k > 0; k--)
			oput('_');
		for ( k = w / t.Char; k > 0; k--)
			oput('\b');
	}
	while (*codep != 0){
		if (*codep & 0200){
			codep = plot(codep);
			oputs(t.plotoff);
			oput(' ');
		} else {
			if (plotmode)
				oputs(t.plotoff);
			/*
			 * simulate bold font as overstrike if no t.bdon
			 */
			if (xfont == 2 && !(*t.bdon & 0377)) {
				oput(*codep);
				oput('\b');
			}
			*obufp++ = *codep++;
			if (obufp == (obuf + OBUFSZ + ascii - 1))
				flusho();
/*			oput(*codep++);*/
		}
	}
	if (!w)
		for ( k = phyw / t.Char; k > 0; k--)
			oput('\b');
	}
}

char *
plot(char *x)
{
	char *j, *k;
	int i;

	if (!plotmode)
		oputs(t.ploton);
	k = x;
	if ((*k & 0377) == 0200)
		k++;
	for (; *k; k++) {
		if (*k & 0200) {
			if (*k & 0100)
				j = *k & 040? t.up : t.down;
			else
				j = *k & 040? t.left : t.right;
			if (!(i = *k & 037))
				return (++k);
			while (i--)
				oputs(j);
		} else
			oput(*k);
	}
	return (k);
}

void
move(void)
{
	char *i, *j, *p, *q;
	int k, iesct, dt;

	iesct = esct;
	i = (esct += esc)? "\0" : "\n\0";
	j = t.hlf;
	p = t.right;
	q = t.down;
	if (lead) {
		if(lead < 0){
			lead = -lead;
			i = t.flr;
		/*	if(!esct)i = t.flr; else i = "\0";*/
			j = t.hlr;
			q = t.up;
		}
		if(*i & 0377){
			k = lead/t.Newline;
			lead = lead%t.Newline;
			while(k--)
				oputs(i);
		}
		if (*j & 0377) {
			k = lead/t.Halfline;
			lead = lead%t.Halfline;
			while(k--)
				oputs(j);
		} else {   /* no half-line forward, not at line beginning */
			k = lead/t.Newline;
			lead = lead%t.Newline;
			if (k > 0)
				esc=esct;
			i = "\n";
			while (k--)
				oputs(i);
		}
	}
	if (esc) {
		if(esc < 0){
			esc = -esc;
			j = "\b";
			p = t.left;
		}else{
			j = " ";
			if(hflg)while((dt = dtab - (iesct%dtab)) <= esc){
				if(dt%t.Em || dt==t.Em)
					break;
				oput(TAB);
				esc -= dt;
				iesct += dt;
			}
		}
		k = esc/t.Em;
		esc = esc%t.Em;
		while (k--)
			oputs(j);
	}
	if((*t.ploton & 0377) && (esc || lead)){
		if(!plotmode)
			oputs(t.ploton);
		esc /= t.Hor;
		lead /= t.Vert;
		while(esc--)
			oputs(p);
		while(lead--)
			oputs(q);
		oputs(t.plotoff);
	}
	esc = lead = 0;
}

void
ptlead(void)
{
	move();
}

void
dostop(void)
{
	char junk;

	flusho();
	read(2, &junk, 1);
}
