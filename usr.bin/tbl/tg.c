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
static char sccsid[] = "@(#)tg.c	4.3 (Berkeley) 4/18/91";
#endif /* not lint */

/* tg.c: process included text blocks */

#include "tbl.h"

gettext(sp, ilin,icol, fn, sz)
char *sp, *fn, *sz;
{
	/* get a section of text */
	char line[256];
	int oname;
	char *vs;
	if (texname==0) error("Too many text block diversions");
	if (textflg==0)
	{
		fprintf(tabout, ".nr %d \\n(.lu\n", SL); /* remember old line length */
		textflg=1;
	}
	fprintf(tabout, ".eo\n");
	fprintf(tabout, ".am %02d\n", icol+80);
	fprintf(tabout, ".br\n");
	fprintf(tabout, ".di %c+\n", texname);
	rstofill();
	if (fn && *fn) fprintf(tabout, ".nr %d \\n(.f\n.ft %s\n", S1, fn);
	fprintf(tabout, ".ft \\n(.f\n"); /* protect font */
	vs = vsize[stynum[ilin]][icol];
	if ((sz && *sz) || (vs && *vs))
	{
		fprintf(tabout, ".nr %d \\n(.v\n", S2);
		if (vs==0 || *vs==0) vs= "\\n(.s+2";
		if (sz && *sz)
		fprintf(tabout, ".ps %s\n",sz);
		fprintf(tabout, ".vs %s\n",vs);
		fprintf(tabout, ".if \\n(%du>\\n(.vu .sp \\n(%du-\\n(.vu\n", S2,S2);
	}
	if (cll[icol][0])
	fprintf(tabout, ".ll %sn\n", cll[icol]);
	else
	fprintf(tabout, ".ll \\n(%du*%du/%du\n",SL,ctspan(ilin,icol),ncol+1);
	fprintf(tabout,".if \\n(.l<\\n(%d .ll \\n(%du\n", icol+CRIGHT, icol+CRIGHT);
	if (ctype(ilin,icol)=='a')
	fprintf(tabout, ".ll -2n\n");
	fprintf(tabout, ".in 0\n");
	while (gets1(line))
	{
		if (line[0]=='T' && line[1]=='}' && line[2]== tab) break;
		if (match("T}", line)) break;
		fprintf(tabout, "%s\n", line);
	}
	if (fn && *fn) fprintf(tabout, ".ft \\n(%d\n", S1);
	if (sz && *sz) fprintf(tabout, ".br\n.ps\n.vs\n");
	fprintf(tabout, ".br\n");
	fprintf(tabout, ".di\n");
	fprintf(tabout, ".nr %c| \\n(dn\n", texname);
	fprintf(tabout, ".nr %c- \\n(dl\n", texname);
	fprintf(tabout, "..\n");
	fprintf(tabout, ".ec \\\n");
	/* copy remainder of line */
	if (line[2])
	tcopy (sp, line+3);
	else
	*sp=0;
	oname=texname;
	texname = texstr[++texct];
	return(oname);
}
untext()
{
	rstofill();
	fprintf(tabout, ".nf\n");
	fprintf(tabout, ".ll \\n(%du\n", SL);
}
