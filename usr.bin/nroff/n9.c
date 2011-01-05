*
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
static char sccsid[] = "@(#)n9.c	4.3 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "tdef.h"

/*
troff9.c

misc functions
*/

extern int cbuf[];
extern int *cp;
extern int ch;
extern int chbits;
extern int dfact;
extern int vflag;
extern int pts;
extern int fc;
extern int padc;
extern int tabtab[];
extern int nlflg;
extern int lss;
extern int tabch, ldrch;
extern int tabc, dotc;
extern int nchar, rchar;
extern int xxx;

setline()
{
	register *i, length, c;
	int w, cnt, delim, rem, temp;

	if((delim = getch()) & MOT)return;
		else delim &= CMASK;
	vflag = 0;
	dfact = EM;
	length = quant(atoi0(),HOR);
	dfact = 1;
	if(!length){
		eat(delim);
		return;
	}
s0:
	if(((c = getch()) & CMASK) == delim){
		ch = c;
		c = 0204 | chbits;
	}else if((c & CMASK) == FILLER)goto s0;
	w = width(c);
	i = cbuf;
	if(length < 0){
		*i++ = makem(length);
		length = -length;
	}
	if(!(cnt = length/w)){
		*i++ = makem(-(temp = ((w-length)/2)));
		*i++ = c;
		*i++ = makem(-(w - length - temp));
		goto s1;
	}
	if(rem = length%w){
		switch(c & CMASK){
			case 0204: /*rule*/
			case 0224: /*underrule*/
			case 0276: /*root en*/
				*i++ = c | ZBIT;
			default:
				*i++ = makem(rem);
		}
	}
	if(cnt){
		*i++ = RPT;
		*i++ = cnt;
		*i++ = c;
	}
s1:
	*i++ = 0;
	eat(delim);
	cp = cbuf;
}

setov(){
	register i, j, k;
	int *p, delim, o[NOV+1], w[NOV+1];

	if((delim = getch()) & MOT)return;
		else delim &= CMASK;
	for(k=0; (k<NOV) && ((j=(i = getch()) & CMASK) != delim) &&
		(j != '\n'); k++){
			o[k] = i;
			w[k] = width(i);
	}
	o[k] = w[k] = 0;
	if(o[0])for(j=1; j;){
		j = 0;
		for(k=1; o[k] ; k++){
			if(w[k-1] < w[k]){
				j++;
				i = w[k];
				w[k] = w[k-1];
				w[k-1] = i;
				i = o[k];
				o[k] = o[k-1];
				o[k-1] = i;
			}
		}
	}else return;
	p = cbuf;
	for(k=0; o[k]; k++){
		*p++ = o[k];
		*p++ = makem(-((w[k]+w[k+1])/2));
	}
	*p++ = makem(w[0]/2);
	*p = 0;
	cp = cbuf;
}
setbra(){
	register i, *j, k;
	int cnt, delim, dwn;

	if((delim = getch()) & MOT)return;
		else delim &= CMASK;
	j = cbuf + 1;
	cnt = 0;
#ifdef NROFF
	dwn = (2*t.Halfline) | MOT | VMOT;
#endif
#ifndef NROFF
	dwn = EM | MOT | VMOT;
#endif
	while(((k = (i = getch()) & CMASK) != delim) && (k != '\n') &&
		(j <= (cbuf+NC-4))){
		*j++ = i | ZBIT;
		*j++ = dwn;
		cnt++;
	}
	if(--cnt < 0)return;
		else if (!cnt){
			ch = *(j-2);
			return;
	}
	*j = 0;
#ifdef NROFF
	*--j = *cbuf = (cnt*t.Halfline) | MOT | NMOT | VMOT;
#endif
#ifndef NROFF
	*--j = *cbuf = (cnt*EM)/2 | MOT | NMOT | VMOT;
#endif
	*--j &= ~ZBIT;
	cp = cbuf;
}
setvline(){
	register i, c, *k;
	int cnt, neg, rem, ver, delim;

	if((delim = getch()) & MOT)return;
		else delim &= CMASK;
	dfact = lss;
	vflag++;
	i = quant(atoi0(),VERT);
	dfact = 1;
	if(!i){
		eat(delim);
		vflag = 0;
		return;
	}
	if(((c = getch()) & CMASK) == delim){
		c = 0337 | chbits;	/*default box rule*/
	}else getch();
	c |= ZBIT;
	neg = 0;
	if(i < 0){
		i = -i;
		neg = NMOT;
	}
#ifdef NROFF
	ver = 2*t.Halfline;
#endif
#ifndef NROFF
	ver = EM;
#endif
	cnt = i/ver;
	rem = makem(i%ver) | neg;
	ver = makem(ver) | neg;
	k = cbuf;
	if(!neg)*k++ = ver;
	if(rem & ~MOTV){
		*k++ = c;
		*k++ = rem;
	}
	while((k < (cbuf+NC-3)) && cnt--){
		*k++ = c;
		*k++ = ver;
	}
	*(k-2) &= ~ZBIT;
	if(!neg)k--;
	*k = 0;
	cp = cbuf;
	vflag = 0;
}

setfield(x)
int x;
{
	register i, j, *fp;
	int length, ws, npad, temp, type;
	int **pp, *padptr[NPP];
	static int fbuf[FBUFSZ];
	int savfc, savtc, savlc;

	if(x == tabch) rchar = tabc | chbits;
	else if(x ==  ldrch) rchar = dotc | chbits;
	temp = npad = ws = 0;
	savfc = fc; savtc = tabch; savlc = ldrch;
	tabch = ldrch = fc = IMP;
	for(j=0;;j++){
		if((tabtab[j] & TMASK)== 0){
			if(x==savfc)prstr("Zero field width.\n");
			j = 0;
			goto rtn;
		}
		v.hp = sumhp();	/* XXX */
		if((length = ((tabtab[j] & TMASK) - v.hp)) > 0 )break;
	}
	type = tabtab[j] & (~TMASK);
	fp = fbuf;
	pp = padptr;
	if(x == savfc){while(1){
		if(((j = (i = getch()) & CMASK)) == padc){
			npad++;
			*pp++ = fp;
			if(pp > (padptr + NPP - 1))break;
			goto s1;
		}else if(j == savfc) break;
			else if(j == '\n'){
				temp = j;
				nlflg = 0;
				break;
			}
		ws += width(i);
	s1:
		*fp++ = i;
		if(fp > (fbuf + FBUFSZ -3))break;
	}
	if(!npad){
		npad++;
		*pp++ = fp;
		*fp++ = 0;
	}
	*fp++ = temp;
	*fp++ = 0;
	temp = i = (j = length-ws)/npad;
	i = (i/HOR)*HOR;
	if((j -= i*npad) <0)j = -j;
	i = makem(i);
	if(temp <0)i |= NMOT;
	for(;npad > 0; npad--){
		*(*--pp) = i;
		if(j){
			j -= HOR;
			(*(*pp)) += HOR;
		}
	}
	cp = fbuf;
	j = 0;
	}else if(type == 0){
	/*plain tab or leader*/
		if((j = width(rchar)) == 0)nchar = 0;
		else{
			nchar = length /j;
			length %= j;
		}
		if(length)j = length | MOT;
		else j = getch0();
	}else{
	/*center tab*/
	/*right tab*/
		while(((j = (i = getch()) & CMASK) != savtc) &&
			(j != '\n') && (j != savlc)){
			ws += width(i);
			*fp++ = i;
			if(fp > (fbuf +FBUFSZ - 3)) break;
		}
		*fp++ = i;
		*fp++ = 0;
		if(type == RTAB)length -= ws;
		else length -= ws/2; /*CTAB*/
		if(((j = width(rchar)) == 0) || (length <= 0))nchar = 0;
		else{
			nchar = length/j;
			length %= j;
		}
		length = (length/HOR)*HOR;
		j = makem(length);
		cp = fbuf;
		nlflg = 0;
	}
rtn:
	fc = savfc; tabch = savtc; ldrch = savlc;
	return(j);
}
