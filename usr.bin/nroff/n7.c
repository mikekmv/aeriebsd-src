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
static char sccsid[] = "@(#)n7.c	4.6 (Berkeley) 5/2/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "tdef.h"

#ifdef NROFF
#define GETCH gettch
#endif
#ifndef NROFF
#define GETCH getch
#endif

/*
troff7.c

text
*/

int brflg;

void donum(void);
void nofill(void);
void callsp(void);
void ckul(void);
void storeline(int, int);
int findn1(int);
int findt(int);
int movword(void);
void storeword(int, int);
int gettch(void);

void
tbreak(void)
{
	int *i, j, pad, res;

	trap = 0;
	if (nb)
		return;
	if ((dip == d) && (v.nl == -1)) {
		newline(1);
		return;
	}
	if (!nc) {
		setnel();
		if (!wch)
			return;
		if (pendw)
			getword(1);
		movword();
	} else if (pendw && !brflg) {
		getword(1);
		movword();
	}
	*linep = dip->nls = 0;
#ifdef NROFF
	if (dip == d)
		horiz(po);
#endif
	if (lnmod)
		donum();
	lastl = ne;
	if (brflg != 1)
		totout = 0;
	else if(ad) {
		if ((lastl = (ll - un)) < ne)
			lastl = ne;
	}
	if (admod && ad && (brflg != 2)) {
		lastl = ne;
		adsp = adrem = 0;
#ifdef NROFF
		if (admod == 1)
			un +=  quant(nel/2,t.Adj);
#else
		if (admod == 1)
			un += nel/2;
#endif
		else if (admod == 2)
			un += nel;
	}

	totout++;
	brflg = 0;
	if ((lastl+un) > dip->maxl)
		dip->maxl = (lastl+un);
	horiz(un);
#ifdef NROFF
	if (adrem%t.Adj)
		res = t.Hor;
	else
		res = t.Adj;
#endif
	for (i = line; nc > 0; ) {
		if (((j = *i++) & CMASK) == ' ') {
			pad = 0;
			do {
				pad += width(j);
				nc--;
			} while(((j = *i++) & CMASK) == ' ');
			i--;
			pad += adsp;
			--nwd;
			if (adrem) {
				if (adrem < 0) {
#ifdef NROFF
					pad -= res;
					adrem += res;
				} else if ((totout & 01) ||
				    adrem / res >= nwd) {
					pad += res;
					adrem -= res;
#else
					pad--;
					adrem++;
				} else {
					pad++;
					adrem--;
#endif
				}
			}
			horiz(pad);
		} else {
			pchar(j);
			nc--;
		}
	}
	if (ic) {
		if ((j = ll - un - lastl + ics) > 0)
			horiz(j);
		pchar(ic);
	}
	if (icf)
		icf++;
	else
		ic = 0;
	ne = nwd = 0;
	un = in;
	setnel();
	newline(0);
	if (dip != d) {
		if (dip->dnl > dip->hnl)
			dip->hnl = dip->dnl;
	} else {
		if (v.nl > dip->hnl)
			dip->hnl = v.nl;
	}
	for (j = ls-1; (j > 0) && !trap; j--)
		newline(0);
	spread = 0;
}

void
donum(void)
{
	int i, nw;

	nrbits = nmbits;
	nw = width('1' | nrbits);
	if (nn) {
		nn--;
		goto d1;
	}
	if (v.ln%ndf) {
		v.ln++;
	d1:
		un += nw*(3+nms+ni);
		return;
	}
	i = 0;
	if (v.ln < 100)
		i++;
	if (v.ln < 10)
		i++;
	horiz(nw * (ni + i));
	nform = 0;
	fnumb(v.ln,pchar);
	un += nw*nms;
	v.ln++;
}

void
text(void)
{
	static int spcnt;
	int i;

	nflush++;
	if ((dip == d) && (v.nl == -1)) {
		newline(1);
		return;
	}
	setnel();
	if (ce || !fi) {
		nofill();
		return;
	}
	if (pendw)
		goto t4;
	if (pendt) {
		if (spcnt)
			goto t2;
		else
			goto t3;
	}
	pendt++;
	if (spcnt)
		goto t2;
	while (((i = GETCH()) & CMASK) == ' ')
		spcnt++;
	if (nlflg) {
	t1:
		nflush = pendt = ch = spcnt = 0;
		callsp();
		return;
	}
	ch = i;
	if(spcnt) {
	t2:
		tbreak();
		if (nc || wch)
			goto rtn;
		un += spcnt*sps;
		spcnt = 0;
		setnel();
		if (trap)
			goto rtn;
		if (nlflg)
			goto t1;
	}
t3:
	if (spread)
		goto t5;
	if (pendw || !wch)
	t4:
		if(getword(0))
			goto t6;
	if (!movword())
		goto t3;
t5:
	if (nlflg)
		pendt = 0;
	adsp = adrem = 0;
	if (ad) {
		adsp = nwd==1? nel : nel / (nwd - 1);
#ifdef NROFF
		adsp = (adsp / t.Adj) * t.Adj;
#endif
		adrem = nel - adsp * (nwd - 1);
	}
	brflg = 1;
	tbreak();
	spread = 0;
	if (!trap)
		goto t3;
	if (!nlflg)
		goto rtn;
t6:
	pendt = 0;
	ckul();
rtn:
	nflush = 0;
}

void
nofill(void)
{
	int i, j;

	if (!pendnf) {
		over = 0;
		tbreak();
		if (trap)
			goto rtn;
		if (nlflg) {
			ch = nflush = 0;
			callsp();
			return;
		}
		adsp = adrem = 0;
		nwd = 10000;
	}

	while ((j = ((i = GETCH()) & CMASK)) != '\n') {
		if (j == ohc)
			continue;
		if (j == CONT) {
			pendnf++;
			nflush = 0;
			flushi();
			ckul();
			return;
		}
		storeline(i,-1);
	}
	if (ce) {
		ce--;
		if ((i = quant(nel / 2, HOR)) > 0)
			un += i;
	}
	if (!nc)
		storeline(FILLER,0);
	brflg = 2;
	tbreak();
	ckul();
rtn:
	pendnf = nflush = 0;
}

void
callsp(void)
{
	int i;

	i = flss? flss : lss;
	flss = 0;
	casesp(i);
}

void
ckul(void)
{
	if (ul && (--ul == 0)) {
		cu = 0;
		font = sfont;
		mchbits();
	}
	if (it && (--it == 0) && itmac)
		control(itmac,0);
}

void
storeline(int c, int w)
{
	int i;

	if ((c & CMASK) == JREG) {
		if ((i=findr(c>>BYTE)) != -1)
			vlist[i] = ne;
		return;
	}
	if (linep >= (line + lnsize - 1)){
		if(!over){
			prstrfl("Line overflow.\n");
			over++;
			c = 0343;
			w = -1;
			goto s1;
		}
		return;
	}
s1:
	if (w == -1)
		w = width(c);
	ne += w;
	nel -= w;
/*
 *	if( cu && !(c & MOT) && (trtab[(c & CMASK)] == ' '))
 *		c = ((c & ~ulbit) & ~CMASK) | '_';
 */
	*linep++ = c;
	nc++;
}

void
newline(int a)
{
	int opn, i, j, nlss = 0;

	if (a)
		goto nl1;
	if (dip != d) {
		j = lss;
		pchar1(FLSS);
		if (flss)
			lss = flss;
		i = lss + dip->blss;
		dip->dnl += i;
		pchar1(i);
		pchar1('\n');
		lss = j;
		dip->blss = flss = 0;
		if (dip->alss) {
			pchar1(FLSS);
			pchar1(dip->alss);
			pchar1('\n');
			dip->dnl += dip->alss;
			dip->alss = 0;
		}
		if (dip->ditrap && !dip->ditf &&
		    dip->dnl >= dip->ditrap && dip->dimac)
			if (control(dip->dimac,0)) {
				trap++;
				dip->ditf++;
			}
		return;
	}
	j = lss;
	if (flss)
		lss = flss;
	nlss = dip->alss + dip->blss + lss;
	v.nl += nlss;
#ifndef NROFF
	if (ascii)
		dip->alss = dip->blss = 0;
#endif
	pchar1('\n');
	flss = 0;
	lss = j;
	if (v.nl < pl)
		goto nl2;
nl1:
	ejf = dip->hnl = v.nl = 0;
	ejl = frame;
	if (donef == 1) {
		if ((!nc && !wch) || ndone)
			done1(0);
		ndone++;
		donef = 0;
		if (frame == stk)
			nflush++;
	}
	opn = v.pn;
	v.pn++;
	if (npnflg) {
		v.pn = npn;
		npn = npnflg = 0;
	}
nlpn:
	if (v.pn == pfrom)
		print++;
	else if (opn == pto || v.pn > pto) {
		print = 0;
		chkpn();
		goto nlpn;
	}

	if (stop && print) {
		dpn++;
		if (dpn >= stop) {
			dpn = 0;
			dostop();
		}
	}
nl2:
	trap = 0;
	if (v.nl == 0) {
		if((j = findn(0)) != NTRAP)
			trap = control(mlist[j],0);
	} else if((i = findt(v.nl - nlss)) <= nlss) {
		if ((j = findn1(v.nl - nlss+i)) == NTRAP) {
			prstrfl("Trap botch.\n");
			done2(-5);
		}
		trap = control(mlist[j],0);
	}
}

int
findn1(int a)
{
	int i, j;

	for (i = 0; i < NTRAP; i++) {
		if (mlist[i]) {
			if ((j = nlistx[i]) < 0)
				j += pl;
			if (j == a)
				break;
		}
	}
	return (i);
}

void
chkpn(void)
{
	pfrom = pto = *(pnp++);
	if (pto == -1) {
		flusho();
		done1(0);
	}
	if (pto == -2) {
		print++;
		pfrom = 0;
		pto = 10000;
	} else if(pto & MOT) {
		print++;
		pto &= ~MOT;
		pfrom = 0;
	}
}

int
findt(int a)
{
	int i, j, k;

	k = 32767;
	if (dip != d) {
		if (dip->dimac && ((i = dip->ditrap -a) > 0))
			k = i;
		return (k);
	}
	for (i = 0; i < NTRAP; i++) {
		if (mlist[i]) {
			if ((j = nlistx[i]) < 0)
				j += pl;
			if ((j -= a)  <=  0)
				continue;
			if (j < k)
				k = j;
		}
	}
	i = pl - a;
	if (k > i)
		k = i;
	return (k);
}

int
findt1(void)
{
	int i;

	i = dip != d? dip->dnl : v.nl;
	return (findt(i));
}

void
eject(struct s *a)
{
	int savlss;

	if (dip != d)
		return;
	ejf++;
	ejl = a? a : frame;
	if (trap)
		return;
e1:
	savlss = lss;
	lss = findt(v.nl);
	newline(0);
	lss = savlss;
	if (v.nl && !trap)
		goto e1;
}

int
movword(void)
{
	int i, w, *wp, savwch, hys;

	over = 0;
	wp = wordp;
	if(!nwd) {
		while (((i = *wp++) & CMASK) == ' ') {
			wch--;
			wne -= width(i);
		}
		wp--;
	}
	if (wne > nel && !hyoff && hyf && (!nwd || nel > 3 * sps) &&
	    (!(hyf & 02) || findt1() > lss))
		hyphen(wp);
	savwch = wch;
	hyp = hyptr;
	nhyp = 0;
	while (*hyp && (*hyp <= wp))
		hyp++;
	while (wch) {
		if ((hyoff != 1) && (*hyp == wp)) {
			hyp++;
			if (!wdstart ||
			   (wp > (wdstart + 1) && wp < wdend &&
			    (!(hyf & 04) || wp < wdend - 1) &&
			    (!(hyf & 010) || wp > wdstart + 2))) {
				nhyp++;
				storeline(IMP,0);
			}
		}
		i = *wp++;
		w = width(i);
		wne -= w;
		wch--;
		storeline(i,w);
	}
	if (nel >= 0) {
		nwd++;
		return (0);
	}
	xbitf = 1;
	hys = width(0200); /*hyphen*/
m1:
	if (!nhyp) {
		if (!nwd)
			goto m3;
		if (wch == savwch)
			goto m4;
	}
	if (*--linep != IMP)
		goto m5;
	if (!(--nhyp))
		if (!nwd)
			goto m2;
	if (nel < hys) {
		nc--;
		goto m1;
	}
m2:
	if (((i = *(linep - 1) & CMASK) != '-') && (i != 0203)) {
		*linep = (*(linep-1) & ~CMASK) | 0200;
	w = width(*linep);
	nel -= w;
	ne += w;
	linep++;
/*
	hsend();
*/
	}
m3:
	nwd++;
m4:
	wordp = wp;
	return(1);
m5:
	nc--;
	w = width(*linep);
	ne -= w;
	nel += w;
	wne += w;
	wch++;
	wp--;
	goto m1;
}

void
horiz(int i)
{
	vflag = 0;
	if (i)
		pchar(makem(i));
}

void
setnel(void)
{
	if (!nc) {
		linep = line;
		if (un1 >= 0) {
			un = un1;
			un1 = -1;
		}
		nel = ll - un;
		ne = adsp = adrem = 0;
	}
}

int
getword(int x)
{
	int i, j, swp, noword;

	noword = 0;
	if (x && pendw) {
		*pendw = 0;
		goto rtn;
	}
	if ((wordp = pendw))
		goto g1;
	hyp = hyptr;
	wordp = word;
	over = wne = wch = 0;
	hyoff = 0;
	while (1) {
		j = (i = GETCH()) & CMASK;
		if (j == '\n') {
			wne = wch = 0;
			noword = 1;
			goto rtn;
		}
		if (j == ohc) {
			hyoff = 1;
			continue;
		}
		if (j == ' ') {
			storeword(i,width(i));	/* XXX */
			continue;
		}
		break;
	}

	swp = widthp;
	storeword(' ' | chbits, -1);
	if (spflg) {
		storeword(' ' | chbits, -1);
		spflg = 0;
	}
	widthp = swp;
g0:
	if (j == CONT) {
		pendw = wordp;
		nflush = 0;
		flushi();
		return(1);
	}
	if(hyoff != 1){
		if(j == ohc){
			hyoff = 2;
			*hyp++ = wordp;
			if(hyp > (hyptr+NHYP-1))hyp = hyptr+NHYP-1;
			goto g1;
		}
		if ((j == '-') || (j == 0203))	/*3/4 Em dash*/
			if (wordp > word+1) {
				hyoff = 2;
				*hyp++ = wordp + 1;
				if (hyp > (hyptr+NHYP-1))
					hyp = hyptr+NHYP-1;
			}
	}
	storeword(i,width(i));	/* XXX */
g1:
	j = (i = GETCH()) & CMASK;
	if (j != ' ') {
		if (j != '\n')
			goto g0;
		j = *(wordp - 1) & CMASK;
		if ((j == '.') || (j == '!') || (j == '?'))
			spflg++;
	}
	*wordp = 0;
rtn:
	wdstart = 0;
	wordp = word;
	pendw = 0;
	*hyp++ = 0;
	setnel();
	return (noword);
}

void
storeword(int c, int w)
{

	if (wordp >= &word[WDSIZE - 1]) {
		if (!over) {
			prstrfl("Word overflow.\n");
			over++;
			c = 0343;
			w = -1;
		} else
			return;
	}

	if (w == -1)
		w = width(c);
	wne += w;
	*wordp++ = c;
	wch++;
}

#ifdef NROFF
int
gettch(void)
{
	extern char trtab[];
	int i, j;

	if (!((i = getch()) & MOT) && (i & ulbit)) {
		j = i & CMASK;
		if (cu && (trtab[j] == ' '))
			i = ((i & ~ulbit)& ~CMASK) | '_';

		if (!cu && j > 32 && j < 0370 && !(*t.codetab[j - 32] & 0200))
			i &= ~ulbit;
	}
	return(i);
}
#endif
