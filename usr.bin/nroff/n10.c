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
static char sccsid[] = "@(#)n10.c	4.6 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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

void
ptinit(void)
{
	extern char *setbrk();

	char *q, **p;
	int i, j, x[8];

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
