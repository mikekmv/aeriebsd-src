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
static char sccsid[] = "@(#)tb.c	4.6 (Berkeley) 4/18/91";
#endif /* not lint */

/* tb.c: check which entries exist, also storage allocation */
# include "t..c"
checkuse()
{
	int i,c, k;
	for(c=0; c<ncol; c++)
	{
		used[c]=lused[c]=rused[c]=0;
		for(i=0; i<nlin; i++)
		{
			if (instead[i] || fullbot[i]) continue;
			k = ctype(i,c);
			if (k== '-' || k == '=') continue;
			if ((k=='n'||k=='a'))
			{
				rused[c]|= real(table[i][c].rcol);
				if( !real(table[i][c].rcol))
				used[c] |= real(table[i][c].col);
				if (table[i][c].rcol)
				lused[c] |= real(table[i][c].col);
			}
			else
			used[c] |= real(table[i][c].col);
		}
	}
}
real( s)
char *s;
{
	if (s==0) return(0);
	if (!point(s)) return(1);
	if (*s==0) return(0);
	return(1);
}
int spcount = 0;
extern char * calloc();
# define MAXVEC 20
char *spvecs[MAXVEC];

char *
chspace() {
	char *pp;
	if (spvecs[spcount])
		return (spvecs[spcount++]);
	if (spcount >= MAXVEC)
		error("Too many characters in table");
	spvecs[spcount++] = pp = calloc(MAXCHS + 200, 1);
	if (pp == 0)
		error("no space for characters");
	return (pp);
}
# define MAXPC 50
char *thisvec;
int tpcount = -1;
char *tpvecs[MAXPC];

int *
alocv(n) {
	int *tp, *q;
	if (tpcount < 0 || thisvec + n > tpvecs[tpcount] + MAXCHS) {
		tpcount++;
		if (tpvecs[tpcount] == 0) {
			tpvecs[tpcount] = calloc(MAXCHS, 1);
		}
		thisvec = tpvecs[tpcount];
		if (thisvec == 0)
			error("no space for vectors");
	}
	tp = (int *) thisvec;
	thisvec += n;
	for (q = tp; q < (int *) thisvec; q++)
		*q = 0;
	return (tp);
}
release()
{
	extern char *exstore;
	/* give back unwanted space in some vectors */
	spcount=0;
	tpcount= -1;
	exstore=0;
}
