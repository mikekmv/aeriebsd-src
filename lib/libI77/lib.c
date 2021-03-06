/*	$Id: lib.c,v 1.1 2011/08/26 00:38:54 mickey Exp $	*/
/*
 * Copyright(C) Caldera International Inc. 2001-2002. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code and documentation must retain the above
 * copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditionsand the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * 	This product includes software developed or owned by Caldera
 *	International, Inc.
 * Neither the name of Caldera International, Inc. nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE
 * FOR ANY DIRECT, INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OFLIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <string.h>

#include "fio.h"

void
setcilist(cilist *x, int u, char *fmt,int rec,int xerr,int end)
{
	x->ciunit=u;
	x->cifmt=fmt;
	x->cirec=rec;
	x->cierr=xerr;
	x->ciend=end;
}

void
setolist(olist *x, int xunit, char *fname, char *sta, char *fm,
    int rl, char *blnk, int oerr)
{
	x->oerr=oerr;
	x->ounit=xunit;
	x->ofnm=fname;
	x->ofnmlen=strlen(fname);
	x->osta=sta;
	x->ofm=fm;
	x->orl=rl;
	x->oblnk=blnk;
}

void
stcllist(cllist *x, int xunit, char *stat, int cerr)
{
	x->cerr=cerr;
	x->cunit=xunit;
	x->csta=stat;
}

void
setalist(alist *x, int xunit, int aerr)
{
	x->aunit=xunit;
	x->aerr=aerr;
}
