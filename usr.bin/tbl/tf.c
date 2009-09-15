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
static char sccsid[] = "@(#)tf.c	4.3 (Berkeley) 4/18/91";
#endif /* not lint */

/* tf.c: save and restore fill mode around table */

#include "tbl.h"

savefill()
{
	/* remembers various things: fill mode, vs, ps in mac 35 (SF) */
	fprintf(tabout, ".de %d\n",SF);
	fprintf(tabout, ".ps \\n(.s\n");
	fprintf(tabout, ".vs \\n(.vu\n");
	fprintf(tabout, ".in \\n(.iu\n");
	fprintf(tabout, ".if \\n(.u .fi\n");
	fprintf(tabout, ".if \\n(.j .ad\n");
	fprintf(tabout, ".if \\n(.j=0 .na\n");
	fprintf(tabout, "..\n");
	fprintf(tabout, ".nf\n");
	/* set obx offset if useful */
	fprintf(tabout, ".nr #~ 0\n");
	fprintf(tabout, ".if n .nr #~ 0.6n\n");
}
rstofill()
{
	fprintf(tabout, ".%d\n",SF);
}
endoff()
{
	int i;
	for(i=0; i<MAXHEAD; i++)
	if (linestop[i])
	fprintf(tabout, ".nr #%c 0\n", 'a'+i);
	for(i=0; i<texct; i++)
	fprintf(tabout, ".rm %c+\n",texstr[i]);
	fprintf(tabout, "%s\n", last);
}
ifdivert()
{
	fprintf(tabout, ".ds #d .d\n");
	fprintf(tabout, ".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n");
}
saveline()
{
	fprintf(tabout, ".if \\n+(b.=1 .nr d. \\n(.c-\\n(c.-1\n");
	linstart=iline;
}
restline()
{
	fprintf(tabout,".if \\n-(b.=0 .nr c. \\n(.c-\\n(d.-%d\n", iline-linstart);
	linstart = 0;
}
cleanfc()
{
	fprintf(tabout, ".fc\n");
}
