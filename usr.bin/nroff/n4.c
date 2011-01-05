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
static char sccsid[] = "@(#)n4.c	4.2 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "tdef.h"

/*
troff4.c

number registers, conversion, arithmetic
*/

int regcnt = NNAMES;

int wrc(int);
int decml(int, int (*)(int));
int roman(int, int (*)(int));
int abc(int, int (*)(int));

void
setn(void)
{
	int i, j, f;

	f = nform = 0;
	if ((i=getch() & CMASK) == '+')f = 1;
		else if(i == '-')f = -1;
			else ch = i;
	if ((i=getsn()) == 0)
		return;
	if ((i & 0177) == '.')
		switch(i>>BYTE){
		case 's': i = pts & 077;	break;
		case 'v': i = lss;		break;
		case 'f': i = font + 1;		break;
		case 'p': i = pl;		break;
		case 't': i = findt1();		break;
		case 'o': i = po;		break;
		case 'l': i = ll;		break;
		case 'i': i = in;		break;
		case '$': i = frame->nargs;	break;
		case 'A': i = ascii;		break;
		case 'c': i = v.cd;		break;
		case 'n': i = lastl;		break;
		case 'a': i = ralss;		break;
		case 'h': i = dip->hnl;		break;
		case 'd':
			if(dip != d)i = dip->dnl; else i = v.nl;
			break;
		case 'u': i = fi;		break;
		case 'j': i = ad + 2*admod;	break;
		case 'w': i = width(*(pinchar-1));		break;	/* XXX */
		case 'x': i = nel;	break;
		case 'y': i = un;		break;
		case 'T': i = dotT;		break; /*-Tterm used in nroff*/
		case 'V': i = VERT;		break;
		case 'H': i = HOR;		break;
		case 'k': i = ne;		break;
		case 'P': i = print;		break;
		case 'L': i = ls;		break;
		case 'R': i = NN - regcnt;	break;
		case 'z': i = dip->curd;
			cbuf[0] = i & BMASK;
			cbuf[1] = (i >> BYTE) & BMASK;
			cbuf[2] = 0;
			cp = cbuf;
			return;
#ifndef NROFF
		case 'b': i = bdtab[font];		break;
#endif

		default:
			goto s0;
	}
	else{
s0:
		if((j=findr(i)) == -1)
			i = 0;
		else {
			i = (vlist[j] = (vlist[j] + inc[j]*f));
			nform = fmt[j];
		}
	}
	setn1(i);
	cp = cbuf;
}

void
setn1(int i)
{
	cp = cbuf;
	nrbits = 0;
	fnumb(i, wrc);
	*cp = 0;
	cp = cbuf;
}

int
findr(int i)
{
	static int numerr;
	int j;

	if (i == 0)
		return(-1);
	for (j = 0; j < NN; j++)
		if (i == r[j])
			break;
	if (j != NN)
		return (j);
	for (j = 0; j < NN; j++) {
		if (r[j] == 0) {
			r[j] = i;
			regcnt++;
			break;
		}
	}
	if (j == NN) {
		if (!numerr)
			prstrfl("Too many number registers.\n");
		if (++numerr > 1)
			done2(04); else edone(04);
	}
	return (j);
}

int
fnumb(int i,int (*f)(int))
{
	int j;

	j = 0;
	if(i < 0) {
		j = (*f)('-' | nrbits);
		i = -i;
	}
	switch(nform){
		default:
		case 0:
		case '1':	return (decml(i,f) + j);
		case 'i':
		case 'I':	return (roman(i,f) + j);
		case 'a':
		case 'A':	return (abc(i,f) + j);
	}
}

int
decml(int i, int (*f)(int))
{
	int j, k;

	k = 0;
	nform--;
	if ((j = i / 10) || (nform > 0))
		k = decml(j, f);
	return (k + (*f)((i % 10 + '0') | nrbits));
}

int
roman0(int i, int (*f)(int), char *onesp, char *fivesp)
{
	int q, rem, k;

	k = 0;
	if (!i)
		return(0);
	k = roman0(i / 10,f, onesp + 1, fivesp + 1);
	q = (i=i%10)/5;
	rem = i%5;
	if (rem == 4){
		k += (*f)(*onesp | nrbits);
		if(q)
			i = *(onesp+1);
		else
			i = *fivesp;
		return (k += (*f)(i | nrbits));
	}
	if(q)
		k += (*f)(*fivesp | nrbits);
	while(--rem >= 0)
		k += (*f)(*onesp | nrbits);
	return(k);
}

int
roman(int i, int (*f)(int))
{

	if (!i)
		return ((*f)('0' | nrbits));
	if (nform == 'i')
		return (roman0(i, f, "ixcmz", "vldw"));
	else
		return (roman0(i, f, "IXCMZ", "VLDW"));
}

int
abc0(int i, int (*f)(int ))
{
	int j, k;

	k = 0;
	if ((j = i / 26))
		k = abc0(j - 1, f);
	return (k + (*f)((i % 26 + nform) | nrbits));
}

int
abc(int i, int (*f)(int))
{
	if (!i)
		return((*f)('0' | nrbits));
	else
		return(abc0(i-1,f));
}

int
wrc(int i)
{
	if (cp >= &cbuf[NC])
		return (0);
	*cp++ = i;
	return (1);
}

long
atoi0(void)
{
	int ii, k, cnt;
	long i, acc;

	i = 0; acc = 0;
	nonumb = 0;
	cnt = -1;
a0:
	cnt++;
	switch ((ii = getch()) & CMASK) {
	default:
		ch = ii;
		if (cnt)
			break;
	case '+':
		i = ckph();
		if (nonumb)
			break;
		acc += i;
		goto a0;
	case '-':
		i = ckph();
		if (nonumb)
			break;
		acc -= i;
		goto a0;
	case '*':
		i = ckph();
		if (nonumb)
			break;
		acc *= i;
		goto a0;
	case '/':
		i = ckph();
		if (nonumb)
			break;
		if (i == 0) {
			prstrfl("Divide by zero.\n");
			acc = 0;
		} else
			acc /= i;
		goto a0;
	case '%':
		i = ckph();
		if (nonumb)
			break;
		acc %= i;
		goto a0;
	case '&':	/*and*/
		i = ckph();
		if (nonumb)
			break;
		acc = (acc > 0) && (i > 0)? 1 : 0;
		goto a0;
	case ':':	/*or*/
		i = ckph();
		if (nonumb)
			break;
		acc = (acc > 0) || (i > 0)? 1 : 0;
		goto a0;
	case '=':
		if (((ii = getch()) & CMASK) != '=')
			ch = ii;
		i = ckph();
		if (nonumb) {
			acc = 0;
			break;
		}
		if (i == acc)
			acc = 1;
		else acc = 0;
		goto a0;
	case '>':
		k = 0;
		if (((ii = getch()) & CMASK) == '=')
			k++;
		else
			ch =ii;
		i = ckph();
		if (nonumb) {
			acc = 0;
			break;
		}
		acc = acc > (i - k)? 1 : 0;
		goto a0;
	case '<':
		k = 0;
		if (((ii=getch()) & CMASK) == '=')
			k++;
		else
			ch = ii;
		i = ckph();
		if (nonumb) {
			acc = 0;
			break;
		}
		acc = acc < (i + k)? 1 : 0;
		goto a0;
	case ')': break;
	case '(':
		acc = atoi0();
		goto a0;
	}

	return (acc);
}

long
ckph(void)
{
	long j;
	int i;

	if (((i = getch()) & CMASK) == '(')
		j = atoi0();
	else {
		ch = i;
		j = atoi1();
	}

	return(j);
}

long
atoi1(void)
{
	int i, j, digits, neg, abs, field;
	long acc;

	neg = abs = field = digits = 0;
	acc = 0;
a0:
	switch ((i = getch()) & CMASK) {
	default:
		ch = i;
		break;
	case '+':
		goto a0;
	case '-':
		neg = 1;
		goto a0;
	case '|':
		abs = 1 + neg;
		neg = 0;
		goto a0;
	}
a1:
	while (((j = ((i = getch()) & CMASK) - '0') >= 0) && (j <= 9)) {
		field++;
		digits++;
		acc = 10*acc + j;
	}
	if((i & CMASK) == '.'){
		field++;
		digits = 0;
		goto a1;
	}
	ch = i;
	if (!field)
		goto a2;
	switch ((i = getch()) & CMASK) {
	case 'u':
		i = j = 1;
		break;
	case 'v':	/*VSs - vert spacing*/
		j = lss;
		i = 1;
		break;
	case 'm':	/*Ems*/
		j = EM;
		i = 1;
		break;
	case 'n':	/*Ens*/
		j = EM;
#ifndef NROFF
		i = 2;
#endif
#ifdef NROFF
		i = 1;	/*Same as Ems in NROFF*/
#endif
		break;
	case 'p':	/*Points*/
		j = INCH;
		i = 72;
		break;
	case 'i':	/*Inches*/
		j = INCH;
		i = 1;
		break;
	case 'c':	/*Centimeters*/
		j = INCH*50;
		i = 127;
		break;
	case 'P':	/*Picas*/
		j = INCH;
		i = 6;
		break;
	default:
		j = dfact;
		ch = i;
		i = dfactd;
	}
	if (neg)
		acc = -acc;
	if  (!noscale)
		acc = (acc * j) / i;
	if ((field != digits) && (digits > 0))
		while (digits--)
			acc /= 10;
	if (abs) {
		if (dip != d)
			j = dip->dnl;
		else
			j = v.nl;
		if (!vflag)
			j = v.hp = sumhp();	/* XXX */
		if (abs == 2)
			j = -j;
		acc -= j;
	}
a2:
	nonumb = !field;
	return(acc);
}

void
caserr(void)
{
	int i,j;

	lgf++;
	while (!skip() && (i=getrq())) {
		for (j = NNAMES; j < NN; j++)	/* NNAMES predefined names */
			if (i == r[j])
				break;

		if (j != NN){
			r[j] = vlist[j] = inc[j] = fmt[j] = 0;
			regcnt--;
		}
	}
}

void
casenr(void)
{
	int i, j;

	lgf++;
	skip();
	if ((i = findr(getrq())) == -1)
		return;
	skip();
	j = inumb(&vlist[i]);
	if (nonumb)
		return;
	vlist[i] = j;
	skip();
	j = atoi0();
	if (nonumb)
		return;
	inc[i] = j;
}

void
caseaf(void)
{
	int i, j, k;

	lgf++;
	if (skip() || !(i = getrq()) || skip())
		return;
	k = 0;
	if (!alph(j=getch())) {
		ch = j;
		while(((j = getch() & CMASK) >= '0') && (j <= '9'))
			k++;
	}
	if (!k)
		k = j;
	fmt[findr(i)] = k & BMASK;
}

int
vnumb(int *i)
{
	vflag++;
	dfact = lss;
	res = VERT;
	return (inumb(i));
}

int
hnumb(int *i)
{
	dfact = EM;
	res = HOR;
	return (inumb(i));
}

int
inumb(int *n)
{
	int i, j, f;

	f = 0;
	if (n) {
		if ((j = (i = getch()) & CMASK) == '+')
			f = 1;
		else if (j == '-')
			f = -1;
		else
			ch = i;
	}
	i = atoi0();
	if (n && f)
		i = *n + f*i;
	i = quant(i,res);
	vflag = 0;
	res = dfactd = dfact = 1;
	if (nonumb)
		i = 0;
	return (i);
}

int
quant(int n, int m)
{
	int i, neg;

	neg = 0;
	if (n < 0) {
		neg++;
		n = -n;
	}
	i = n/m;
	if ((n - m*i) > (m/2))
		i += 1;
	i *= m;
	if (neg)
		i = -i;
	return(i);
}
