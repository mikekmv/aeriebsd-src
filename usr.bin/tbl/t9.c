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
static char sccsid[] = "@(#)t9.c	4.4 (Berkeley) 4/18/91";
#endif /* not lint */

/* t9.c: write lines for tables over 200 lines */

#include "tbl.h"

static useln;
yetmore()
{
	for(useln=0; useln<MAXLIN && table[useln]==0; useln++);
	if (useln>=MAXLIN)
	error("Weird.  No data in table.");
	table[0]=table[useln];
	for(useln=nlin-1; useln>=0 && (fullbot[useln] || instead[useln]); useln--);
	if (useln<0)
	error("Weird.  No real lines in table.");
	domore(leftover);
	while (gets1(cstore=cspace) && domore(cstore))
	;
	last =cstore;
	return;
}
domore( dataln)
char *dataln;
{
	int icol, ch;
	if (prefix(".TE", dataln))
	return(0);
	if (dataln[0] == '.' && !isdigit(dataln[1]))
	{
		puts(dataln);
		return(1);
	}
	instead[0]=fullbot[0]=0;
	if (dataln[1]==0)
	switch(dataln[0])
	{
		case '_': fullbot[0]= '-'; putline(useln,0); return(1);
		case '=': fullbot[0]= '='; putline(useln, 0); return(1);
	}
	for (icol = 0; icol <ncol; icol++)
	{
		table[0][icol].col = dataln;
		table[0][icol].rcol=0;
		for(; (ch= *dataln) != '\0' && ch != tab; dataln++)
		;
		*dataln++ = '\0';
		switch(ctype(useln,icol))
		{
			case 'n':
			table[0][icol].rcol = maknew(table[0][icol].col);
			break;
			case 'a':
			table[0][icol].rcol = table[0][icol].col;
			table[0][icol].col= "";
			break;
		}
		while (ctype(useln,icol+1)== 's') /* spanning */
		table[0][++icol].col = "";
		if (ch == '\0') break;
	}
	while (++icol <ncol)
	table[0][icol].col = "";
	putline(useln,0);
	return(1);
}
