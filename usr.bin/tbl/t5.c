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
static char sccsid[] = "@(#)t5.c	4.4 (Berkeley) 4/18/91";
#endif /* not lint */

/* t5.c: read data for table */
# include "t..c"
gettbl()
{
	int icol, ch;
	cstore=cspace= chspace();
	textflg=0;
	for (nlin=nslin=0; gets1(cstore); nlin++)
	{
		stynum[nlin]=nslin;
		if (prefix(".TE", cstore))
		{
			leftover=0;
			break;
		}
		if (prefix(".TC", cstore) || prefix(".T&", cstore))
		{
			readspec();
			nslin++;
		}
		if (nlin>=MAXLIN)
		{
			leftover=(int)cstore;
			break;
		}
		fullbot[nlin]=0;
		if (cstore[0] == '.' && !isdigit(cstore[1]))
		{
			instead[nlin] = cstore;
			while (*cstore++);
			continue;
		}
		else instead[nlin] = 0;
		if (nodata(nlin))
		{
			if (ch = oneh(nlin))
			fullbot[nlin]= ch;
			nlin++;
			nslin++;
			instead[nlin]=(char *)0;
			fullbot[nlin]=0;
		}
		table[nlin] = (struct colstr *)alocv((ncol+2)*sizeof(table[0][0]));
		if (cstore[1]==0)
		switch(cstore[0])
		{
			case '_': fullbot[nlin]= '-'; continue;
			case '=': fullbot[nlin]= '='; continue;
		}
		stynum[nlin] = nslin;
		nslin = min(nslin+1, nclin-1);
		for (icol = 0; icol <ncol; icol++)
		{
			table[nlin][icol].col = cstore;
			table[nlin][icol].rcol=0;
			ch=1;
			if (match(cstore, "T{")) /* text follows */
			table[nlin][icol].col =
			(char *)gettext(cstore, nlin, icol,
					font[stynum[nlin]][icol],
					csize[stynum[nlin]][icol]);
			else
			{
				for(; (ch= *cstore) != '\0' && ch != tab; cstore++)
				;
				*cstore++ = '\0';
				switch(ctype(nlin,icol)) /* numerical or alpha, subcol */
				{
					case 'n':
					table[nlin][icol].rcol =
					(char *)maknew(table[nlin][icol].col);
					break;
					case 'a':
					table[nlin][icol].rcol = table[nlin][icol].col;
					table[nlin][icol].col = "";
					break;
				}
			}
			while (ctype(nlin,icol+1)== 's') /* spanning */
			table[nlin][++icol].col = "";
			if (ch == '\0') break;
		}
		while (++icol <ncol+2)
		{
			table[nlin][icol].col = "";
			table [nlin][icol].rcol=0;
		}
		while (*cstore != '\0')
		cstore++;
		if (cstore-cspace > MAXCHS)
		cstore = cspace = chspace();
	}
	last = cstore;
	permute();
	if (textflg) untext();
	return;
}
nodata(il)
{
	int c;
	for (c=0; c<ncol;c++)
	{
		switch(ctype(il,c))
		{
			case 'c': case 'n': case 'r': case 'l': case 's': case 'a':
			return(0);
		}
	}
	return(1);
}
oneh(lin)
{
	int k, icol;
	k = ctype(lin,0);
	for(icol=1; icol<ncol; icol++)
	{
		if (k != ctype(lin,icol))
		return(0);
	}
	return(k);
}
# define SPAN "\\^"
permute()
{
	int irow, jcol, is;
	char *start, *strig;
	for(jcol=0; jcol<ncol; jcol++)
	{
		for(irow=1; irow<nlin; irow++)
		{
			if (vspand(irow,jcol,0))
			{
				is = prev(irow);
				if (is<0)
				error("Vertical spanning in first row not allowed");
				start = table[is][jcol].col;
				strig = table[is][jcol].rcol;
				while (irow<nlin &&vspand(irow,jcol,0))
				irow++;
				table[--irow][jcol].col = start;
				table[irow][jcol].rcol = strig;
				while (is<irow)
				{
					table[is][jcol].rcol =0;
					table[is][jcol].col= SPAN;
					is = next(is);
				}
			}
		}
	}
}
vspand(ir,ij,ifform)
{
	if (ir<0) return(0);
	if (ir>=nlin)return(0);
	if (instead[ir]) return(0);
	if (ifform==0 && ctype(ir,ij)=='^') return(1);
	if (table[ir][ij].rcol!=0) return(0);
	if (fullbot[ir]) return(0);
	return(vspen(table[ir][ij].col));
}
vspen( s)
char *s;
{
	if (s==0) return(0);
	if (!point(s)) return(0);
	return(match(s, SPAN));
}
