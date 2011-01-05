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
static char sccsid[] = "@(#)n3.c	4.5 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include <ctype.h>
#include <unistd.h>

#include "tdef.h"

/*
troff3.c

macro and string routines, storage allocation
*/

unsigned blist[NBLIST];
int raw2;
int pagech = '%';
int strflg;
int *wbuf;
int *rbuf;
int Buf[NBLIST*BLK + NEV*EVS];

int blisti(filep);
int copyb(void);
char *kvt(int, char *);
int hseg(int (*)(int), filep);

void
caseig(void)
{
	int i;

	offset = 0;
	if ((i = copyb()) != '.')
		control(i, 1);
}

void
casern(void)
{
	register int i, j;

	lgf++;
	skip();
	if (((i = getrq()) == 0) || ((oldmn = findmn(i)) < 0))
		return;
	skip();
	clrmn(findmn(j = getrq()));
	if (j)
		contab[oldmn].rq = (contab[oldmn].rq & MMASK) | j;
}

void
caserm(void)
{
	lgf++;
	while(!skip())
		clrmn(findmn(getrq()));
}

void
caseas(void)
{
	app++;
	caseds();
}

void
caseds(void)
{
	ds++;
	casede();
}

void
caseam(void)
{
	app++;
	casede();
}

void
casede(void)
{
	filep savoff;
	int i, req;

	if (dip != d)
		wbfl();
	req = '.';
	lgf++;
	skip();
	if ((i = getrq()) == 0)
		goto de1;
	if ((offset = finds(i)) == 0)
		goto de1;
	if (ds)
		copys();
	else
		req = copyb();
	wbfl();
	clrmn(oldmn);
	if (newmn)
		contab[newmn].rq = i | MMASK;
	if (apptr) {
		savoff = offset;
		offset = apptr;
		wbt(IMP);
		offset = savoff;
	}
	offset = dip->op;
	if (req != '.')
		control(req,1);
de1:
	ds = app = 0;
	return;
}

int
findmn(int i)
{
	int j;

	for (j = 0; j < NM; j++)
		if (i == (contab[j].rq & ~MMASK))
			break;
	if (j == NM)
		j = -1;
	return(j);
}

void
clrmn(int i)
{
	extern filep boff();
	if(i >= 0){
		if(contab[i].rq & MMASK)ffree(((filep)contab[i].x.mx)<<BLKBITS);
		contab[i].rq = 0;
		contab[i].x.mx = 0;
	}
}

filep
finds(int mn)
{
	filep savip;
	int i;

	oldmn = findmn(mn);
	newmn = 0;
	apptr = (filep)0;
	if (app && (oldmn >= 0) && (contab[oldmn].rq & MMASK)) {
			savip = ip;
			ip = (((filep)contab[oldmn].x.mx)<<BLKBITS);
			oldmn = -1;
			while((i=rbf()) != 0)
				;
			apptr = ip;
			if(!diflg)
				ip = incoff(ip);
			nextb = ip;
			ip = savip;
	} else {
		for(i=0;i<NM;i++)
			if(contab[i].rq == 0)
				break;
		if((i==NM) || (nextb = alloc()) == 0) {
			app = 0;
			if (macerr++ > 1)
				done2(02);
			prstr("Too many string/macro names.\n");
			edone(04);
			return(offset = 0);
		}
		contab[i].x.mx = (unsigned)(nextb>>BLKBITS);
		if(!diflg){
			newmn = i;
			if(oldmn == -1)
				contab[i].rq = -1;
		}else{
			contab[i].rq = mn | MMASK;
		}
	}

	app = 0;
	return(offset = nextb);
}

int
skip(void)
{
	int i;

	while (((i=getch()) & CMASK) == ' ')
		;
	ch = i;
	return (nlflg);
}

int
copyb(void)
{
	int i, j, k, ii, req, state;
	filep savoff;

	if (skip() || !(j=getrq()))
		j = '.';
	req = j;
	k = j>>BYTE;
	j &= BMASK;
	copyf++;
	flushi();
	nlflg = 0;
	state = 1;
	savoff = 0;
	while(1) {
		i = (ii = getch()) & CMASK;
		if (state == 3) {
			if(i == k)
				break;
			if(!k){
				ch = ii;
				i = getach();
				ch = ii;
				if(!i)break;
			}
			state = 0;
			goto c0;
		}
		if (i == '\n') {
			state = 1;
			nlflg = 0;
			goto c0;
		}
		if ((state == 1) && (i == '.')){
			state++;
			savoff = offset;
			goto c0;
		}
		if((state == 2) && (i == j)){
			state++;
			goto c0;
		}
		state = 0;
c0:
		if(offset)
			wbf(ii);
	}
	if (offset) {
		wbfl();
		offset = savoff;
		wbt(0);
	}
	copyf--;
	return(req);
}

void
copys(void)
{
	int i;

	copyf++;
	if (skip())
		goto c0;
	if (((i = getch()) & CMASK) != '"')
		wbf(i);
	while (((i=getch()) & CMASK) != '\n')
		wbf(i);
c0:
	wbt(0);
	copyf--;
}

filep
alloc(void)
{
	filep j;
	int i;

	for (i = 0; i < NBLIST; i++)
		if (blist[i] == 0)
			break;
	if (i==NBLIST)
		j = 0;
	else {
		blist[i] = -1;
		if ((j = boff(i)) < NEV*EVS)
			j = 0;
	}
	return (nextb = j);
}

void
ffree(filep i)
{
	int j;

	while((blist[j = blisti(i)]) != -1){
		i = ((filep)blist[j])<<BLKBITS;
		blist[j] = 0;
	}
	blist[j] = 0;
}

filep
boff(int i)
{
	return (((filep)i)*BLK + NEV*EVS);
}

void
wbt(int i)
{
	wbf(i);
	wbfl();
}

void
wbf(int i)
{
	int j;

	if(!offset)
		return;
	if(!woff){
		woff = offset;
		wbuf = &Buf[woff];
		wbfi = 0;
	}
	wbuf[wbfi++] = i;
	if(!((++offset) & (BLK-1))){
		wbfl();
		if(blist[j = blisti(--offset)] == -1){
			if(alloc() == 0){
				prstr("Out of temp file space.\n");
				done2(01);
			}
			blist[j] = (unsigned)(nextb>>BLKBITS);
		}
		offset = ((filep)blist[j])<<BLKBITS;
	}
	if(wbfi >= BLK)
		wbfl();
}

void
wbfl(void)
{
	if(woff == 0)
		return;
	if((woff & (~(BLK-1))) == (roff & (~(BLK-1))))
		roff = -1;
	woff = 0;
}

int
blisti(filep i)
{
	return((i-NEV*EVS)/(BLK));
}

int
rbf(void)
{
	int i;

	if ((i = rbf0(ip)) == 0) {
		if(!app)
			i = popi();
	} else
		ip = incoff(ip);

	return(i);
}

int
rbf0(filep p)
{
	filep i;

	if ((i = (p & (~(BLK-1)))) != roff) {
		roff = i;
		rbuf = &Buf[roff];
	}
	return (rbuf[p & (BLK - 1)]);
}

filep
incoff(filep p)
{
	filep j;
	int i;

	if (!((j = (++p)) & (BLK-1))){
		if((i = blist[blisti(--p)]) == -1){
			prstr("Bad storage allocation.\n");
			done2(-5);
		}
		j = ((filep)i)<<BLKBITS;
	}
	return(j);
}

int
popi(void)
{
	struct s *p;

	if(frame == stk)
		return(0);
	if(strflg)
		strflg--;
	p = nxf = frame;
	p->nargs = 0;
	frame = p->pframe;
	ip = p->pip;
	nchar = p->pnchar;
	rchar = p->prchar;
	pendt = p->ppendt;
	ap = p->pap;
	cp = p->pcp;
	ch0 = p->pch0;

	return(p->pch);
}

/*
 *	test that the end of the allocation is above a certain location
 *	in memory
 */
#define SPACETEST(base, size) while ((enda - (size)) <= (char *)(base)){setbrk(DELTA);}

int
pushi(filep newip)
{
	struct s *p;

	SPACETEST(nxf, sizeof(struct s));
	p = nxf;
	p->pframe = frame;
	p->pip = ip;
	p->pnchar = nchar;
	p->prchar = rchar;
	p->ppendt = pendt;
	p->pap = ap;
	p->pcp = cp;
	p->pch0 = ch0;
	p->pch = ch;
	cp = ap = 0;
	nchar = rchar = pendt = ch0 = ch = 0;
	frame = nxf;
	if (nxf->nargs == 0)
		nxf += 1;
	else
		nxf = (struct s *)argtop;
	return(ip = newip);
}


char *
setbrk(int x)
{
	char	*p;

	x += sizeof(int) - 1;
	x &= ~(sizeof(int) - 1);
	if ((u_int)(p = sbrk(x)) == -1) {
		prstrfl("Core limit reached.\n");
		edone(0100);
	} else
		enda = p + x;

	return (p);
}

int
getsn(void)
{
	int i;

	if ((i = getach()) == 0)
		return(0);
	if (!raw2 && (i == '[')) {
		while ((i = getach()) && i != ']')
			;
		return (0);
	}
	if (i == '(')
		return getrq();
	else
		return(i);
}

int
setstr(void)
{
	int i;

	lgf++;
	if ((i = getsn()) == 0 || (i = findmn(i)) == -1 ||
	    !(contab[i].rq & MMASK)) {
		lgf--;
		return(0);
	} else {
		SPACETEST(nxf, sizeof(struct s));
		nxf->nargs = 0;
		strflg++;
		lgf--;
		return(pushi(((filep)contab[i].x.mx)<<BLKBITS));
	}
}

typedef	int	tchar;
#define	cbits(x)	((x) & CMASK)

void
collect(void)
{
	tchar i, *strp, *lim, **argpp, **argppend;
	struct s *savnxf;
	int	j, quote;

	copyf++;
	nxf->nargs = 0;
	savnxf = nxf;
	if (skip())
		goto rtn;

	{
		char *memp;
		memp = (char *)savnxf;
		/*
		 *	1 s structure for the macro descriptor
		 *	APERMAC tchar *'s for pointers into the strings
		 *	space for the tchar's themselves
		 */
		memp += sizeof(struct s);
		/*
		 *	CPERMAC (the total # of characters for ALL arguments)
		 *	to a macros, has been carefully chosen
		 *	so that the distance between stack frames is < DELTA
		 */
#define	CPERMAC	200
#define	APERMAC	9
		memp += APERMAC * sizeof(tchar *);
		memp += CPERMAC * sizeof(tchar);
		nxf = (struct s*)memp;
	}
	lim = (tchar *)nxf;
	argpp = (tchar **)(savnxf + 1);
	argppend = &argpp[APERMAC];
	SPACETEST(argppend, sizeof(tchar *));
	strp = (tchar *)argppend;
	/*
	 *	Zero out all the string pointers before filling them in.
	 */
	for (j = 0; j < APERMAC; j++){
		argpp[j] = (tchar *)0;
	}
#if 0
	fprintf(stderr, "savnxf=0x%x,nxf=0x%x,argpp=0x%x,strp=argppend=0x%x,lim=0x%x,enda=0x%x\n",
		savnxf, nxf, argpp, strp, lim, enda);
#endif
	strflg = 0;
	while ((argpp != argppend) && (!skip())) {
		*argpp++ = strp;
		quote = 0;
		if (cbits(i = getch()) == '"')
			quote++;
		else
			ch = i;
		while (1) {
			i = getch();
			if (nlflg ||  (!quote && cbits(i) == ' '))
				break;
			if (quote && cbits(i) == '"' &&
			    cbits(i = getch()) != '"') {
				ch = i;
				break;
			}
			*strp++ = i;
			if (strflg && (strp >= lim)) {
#if 0
				fprintf(stderr, "strp=0x%x, lim = 0x%x\n",
					strp, lim);
#endif
				prstrfl("Macro argument too long.\n");
				copyf--;
				edone(004);
			}
			SPACETEST(strp, 3 * sizeof(tchar));
		}
		*strp++ = 0;
	}
	nxf = savnxf;
	nxf->nargs = argpp - (tchar **)(savnxf + 1);
	argtop = strp;
rtn:
	copyf--;
}

void
seta(void)
{
	int i;

	if ((i = (getch() & CMASK) - '0') > 0 &&
	    i <= APERMAC && i <= frame->nargs)
		ap = *((int **)frame + i-1 + sizeof(struct s)/sizeof(int **));
}

void
caseda(void)
{
	app++;
	casedi();
}

void
casedi(void)
{
	int i, j, *k;

	lgf++;
	if (skip() || ((i = getrq()) == 0)) {
		if (dip != d)
			wbt(0);
		if (dilev > 0) {
			v.dn = dip->dnl;
			v.dl = dip->maxl;
			dip = &d[--dilev];
			offset = dip->op;
		}
		goto rtn;
	}
	if (++dilev == NDI) {
		--dilev;
		prstr("Cannot divert.\n");
		edone(02);
	}
	if (dip != d)
		wbt(0);
	diflg++;
	dip = &d[dilev];
	dip->op = finds(i);
	dip->curd = i;
	clrmn(oldmn);
	k = (int *)&dip->dnl;
	for (j = 0; j < 10; j++)
		k[j] = 0;	/*not op and curd*/
rtn:
	app = 0;
	diflg = 0;
}

void
casedt(void)
{
	lgf++;
	dip->dimac = dip->ditrap = dip->ditf = 0;
	skip();
	dip->ditrap = vnumb((int *)0);
	if (nonumb)
		return;
	skip();
	dip->dimac = getrq();
}

void
casetl(void)
{
	filep begin;
	int i, j = 0, w1, w2, w3, delim;

	dip->nls = 0;
	skip();
	if (dip != d)
		wbfl();
	if ((offset = begin = alloc()) == 0)
		return;
	if ((delim = getch()) & MOT) {
		ch = delim;
		delim = '\'';
	} else
		delim &= CMASK;
	if (!nlflg)
		while (((i = getch()) & CMASK) != '\n') {
			if((i & CMASK) == delim)i = IMP;
			wbf(i);
		}
	wbf(IMP);
	wbf(IMP);
	wbt(0);

	w1 = hseg(width, begin);
	w2 = hseg(width, 0);
	w3 = hseg(width, 0);
	offset = dip->op;
#ifdef NROFF
	if (!offset)
		horiz(po);
#endif
	hseg(pchar, begin);
	if (w2 || w3)
		horiz(j = quant((lt - w2) / 2 - w1, HOR));
	hseg(pchar, 0);
	if (w3) {
		horiz(lt-w1-w2-w3-j);
		hseg(pchar, 0);
	}
	newline(0);
	if (dip != d) {
		if(dip->dnl > dip->hnl)
			dip->hnl = dip->dnl;
	} else {
		if (v.nl > dip->hnl)
			dip->hnl = v.nl;
	}
	ffree(begin);
}

void
casepc(void)
{
	pagech = chget(IMP);
}

int
hseg(int (*f)(int), filep p)
{
	static filep q;
	int acc, i;

	acc = 0;
	if (p)
		q = p;
	while(1){
		i = rbf0(q);
		q = incoff(q);
		if (!i || (i == IMP))
			return (acc);
		if ((i & CMASK) == pagech) {
			nrbits = i & ~CMASK;
			nform = fmt[findr('%')];
			acc += fnumb(v.pn,f);
		} else
			acc += (*f)(i);
	}
}

void
casepm(void)
{
	int i, k, xx, cnt, kk, tot;
	char pmline[10];
	char *p;
	filep j;

	kk = cnt = 0;
	tot = !skip();
	for(i = 0; i<NM; i++){
		if (!((xx = contab[i].rq) & MMASK))
			continue;
		p = pmline;
		j = (((filep)contab[i].x.mx)<<BLKBITS);
		k = 1;
		while ((j = blist[blisti(j)]) != -1) {
			k++;
			j <<= BLKBITS;
		}
		cnt++;
		kk += k;
		if (!tot) {
			*p++ = xx & 0177;
			if (!(*p++ = (xx >> BYTE) & 0177))
				*(p-1) = ' ';
			*p++ = ' ';
			kvt(k,p);
			prstr(pmline);
		}
	}
	if (tot || (cnt > 1)) {
		kvt(kk,pmline);
		prstr(pmline);
	}
}

char *
kvt(int k, char *p)
{
	if (k >= 100)
		*p++ = k/100 + '0';
	if (k >= 10)
		*p++ = (k%100)/10 + '0';
	*p++ = k%10 + '0';
	*p++ = '\n';
	*p = 0;
	return(p);
}
