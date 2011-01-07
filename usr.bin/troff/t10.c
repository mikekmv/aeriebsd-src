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
static char sccsid[] = "@(#)t10.c	4.2 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include "tdef.h"

/*
troff10.c

CAT interface
*/

int mrail;	/*0=LR,1=UR*/
int mmag = 1;	/*0=UM,1=LM*/
int papflg;

void
ptinit(void)
{
	if (ascii || gflag)
		return;
	oput(T_INIT);
	esc = T_IESC;
	ptesc();
	esct = 0;
	esc = po;
	oput(0140); /*some initial lead*/
}

void
ptout(int i)
{
	int *k, lw, *j;
	int ds, de, inith, temp, *slp, dv;
	int psl[16];

	if ((i & CMASK) != '\n') {
		*olinep++ = i;
		return;
	}
	if (olinep == oline) {
		lead += lss;
		return;
	}
	pslp = psl;
	*pslp = lw = inith = dv = 0;
	for(k=oline; k<olinep; k++){
		trflg++;
		xbitf = 1;
		lw += width(*k);
		if((*k & (MOT | VMOT)) == (MOT | VMOT)){
			temp = *k & ~MOTV;
			if(*k & NMOT)temp = -temp;
			dv += temp;
		}
		if(!(*k & MOT) && xflg)for(j=psl; j<=pslp; j++){
			if(xpts == *j)break;
			if(j == pslp){
				*j = xpts;
				*++pslp = 0;
				break;
			}
		}
	}
	if(dv){
		vflag++;
		*olinep++ = makem(-dv);
		vflag = 0;
	}
	if(xflg){
	--pslp;
		for(j=psl; j<=pslp; j++){
			if(*j == mpts){
				temp = *j;
				*j = *pslp;
				*pslp = temp;
				break;
			}
		}
	}
	for(k=oline; k<olinep; k++){
		if(!(*k & MOT) || (*k & VMOT))break;
		*k &= ~MOT;
		if(*k & NMOT){
			*k &= ~NMOT;
			*k = -*k;
		}
		inith += *k;
	}
	lead += dip->blss + lss;
	dip->blss = 0;
	slp = k;
scan:
	temp = esct - po;
	if(mpts & DBL)temp -= 55;
	ds = temp - inith;
	de = lw - temp;
	if(de >= ds){
		back = 0;
		esc = -ds;
		for(k=slp; k<olinep; k++)ptout0(*k);
	}else{
		back = 1;
		esc = de;
		for(k = olinep-1; k>=slp; --k)ptout0(*k);
	}
	if(xflg && (--pslp >= psl))goto scan;
	olinep = oline;
	lead += dip->alss;
	dip->alss = 0;
}

void
ptout0(int i)
{
	int j, k, w, z;

	z = i & ZBIT;

	if (i & MOT) {
		j = i & ~MOTV;
		if (i & NMOT)
			j = -j;
		if (back)
			j = -j;
		if (i & VMOT)
			lead += j;
		else
			esc += j;
		return;
	}
	xbitf = 2;
	if ((i >> BYTE) == oldbits) {
		xfont = pfont;
		xpts = ppts;
		xbitf = 0;
	} else
		xbits(i);
	if ((k = (i & CMASK)) < 040)
		return;

	w = getcw(k-32);
	if (cs) {
		if (bd)
			w += bd - 1;
		j = (cs-w)/2;
		w = cs - j;
		if (bd)
			w -= bd - 1;
	} else
		j = 0;

	if (i & ZBIT)
		w = cs? -j : 0;

	if (back) {
		k = j;
		j = -w;
		w = -k;
	}
	esc += j;
	if ((!xflg || (xpts == *pslp)) && (code & 077)) {
		if (code & 0200) {
			if (smnt)
				xfont = smnt -1;
			else
				goto p1;
		}
		if ((k = (code >> 6) & 01) ^ mcase)
			oput((mcase = k) + 0105);

		if (xfont != mfont) {
			mfont = xfont;
			if (mrail != (xfont&01))
				oput(0101 + (mrail=xfont&01));
			if (mmag != (xfont<2))
				oput(0103 + (mmag=(xfont<2)));
		}
		if (xpts != mpts)
			ptps();
		if (lead)
			ptlead();
		if (esc)
			ptesc();
/*
		oput(code & 077);
*/
		*obufp++ = code & 077;
		if (obufp == (obuf + OBUFSZ + ascii - 1))
			flusho();
		if (bd) {
			bd -= 1;
			if (back && !z)
				bd = -bd;
			if (esc += bd)
				ptesc();
			oput(code & 077);
			if (z)
				esc -= bd;
		}
	} else if(bd && !z) {
		bd -= 1;
		if (back)
			bd = -bd;
		esc += bd;
	}
p1:
	esc += w;
	return;
}

void
ptps(void)
{
	int i, j, k;

	if (psflg)
		return;
	if (cps) {
		psflg++;
		i = findps(cps);
	} else
		i = xpts;
	for (j = 0; (i & 077) > (k = pstab[j]); j++)
		if (!k) {
			k = pstab[--j];
			break;
		}
	j = psctab[j];
	oput((j & ~0200) | 0120);
	if ((!(mpts & DBL))^(!(j & 0200))){
		k = j & 0200? 55 : -55;
		esc += k;
	}
	mpts = i;
}

void
ptlead(void)
{
	int i, k;

	if ((k = lead < 0))
		lead = -lead;
	if (k ^ verm)
		oput(0112 + ((verm = k) << 1));
	if (((k = lead) % 3) == 2)
		k++;
	k /= 3;
	while (k > 0) {
		if ((i=31) > k)
			i = k;
		if (verm)
			paper -= i;
		else
			paper += i;
		oput(((~i) & 037) | 0140);
		if ((paper > (11 * 144 * 15)) && !papflg && ptid != 1) {
			prstr("Excessive paper use.\n");
			papflg++;
			if (ptid != 1) {
				lead = 0;
				done2(0200);
			}
		}
		k -= i;
	}
	lead = 0;
}

void
ptesc(void)
{
	int i, j, k;

	if ((k = esc) < 0)
		esc = -esc;
	if (k ^ escm)
		oput(0107 + (escm = k));
	k = esc;
	while (k > 0) {
		if ((i=127) > k)
			i = k;
		if (((j = (esct + i * (1 - 2 * escm))) > 46 * 72 + 18 - T_IESC) ||
		   j < 0)
			break;
/*
		oput(~i);
*/
		*obufp++ = ~i;
		if (obufp == (obuf + OBUFSZ + ascii - 1))
			flusho();
		esct = j;
		k -= i;
	}
	esc = 0;
}

void
dostop(void)
{
	int i;

	if (ascii)
		return;
	if (!nofeed && !gflag)
		lead += TRAILER;
	ptlead();
	flusho();
	oput(T_INIT);
	oput(T_STOP);
	if (gflag) {
		oput('f');
		for (i = 0; i<4; i++) {
			oput(fontlab[i] & BMASK);
			oput((fontlab[i] >> BYTE) & BMASK);
		}
	} else
		for (i = 8; i > 0; i--)
			oput(T_PAD);
	flusho();
	if (stopmesg)
		prstr("Pages finished.\n");
	mcase = mpts = mfont = mrail = verm = escm = 0;
	mmag = 1;
	report();
	paper = 0;
	esc = T_IESC;
	ptesc();
	esct = 0;
	esc = po;
}
