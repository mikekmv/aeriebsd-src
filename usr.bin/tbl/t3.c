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
static char sccsid[] = "@(#)t3.c	4.3 (Berkeley) 4/18/91";
#endif /* not lint */

/* t3.c: interpret commands affecting whole table */

#include "tbl.h"

struct optstr {
	char *optnam;
	int *optadd;
} options[] = { "expand", &expflg, "EXPAND", &expflg, "center", &ctrflg,
		"CENTER", &ctrflg, "box", &boxflg, "BOX", &boxflg, "allbox", &allflg,
		"ALLBOX", &allflg, "doublebox", &dboxflg, "DOUBLEBOX", &dboxflg,
		"frame", &boxflg, "FRAME", &boxflg, "doubleframe", &dboxflg,
		"DOUBLEFRAME", &dboxflg, "tab", &tab, "TAB", &tab, "linesize",
		&linsize, "LINESIZE", &linsize, "delim", &delim1, "DELIM", &delim1, 0,
		0 };
getcomm()
{
	char line[200], *cp, nb[25], *t;
	struct optstr *lp;
	int c, ci, found;
	for(lp= options; lp->optnam; lp++)
	*(lp->optadd) = 0;
	texname = texstr[texct=0];
	tab = '\t';
	printf(".nr %d \\n(.s\n", LSIZE);
	gets1(line);
	/* see if this is a command line */
	if (index(line,';') == NULL)
	{
		backrest(line);
		return;
	}
	for(cp=line; (c = *cp) != ';'; cp++)
	{
		if (!letter(c)) continue;
		found=0;
		for(lp= options; lp->optadd; lp++)
		{
			if (prefix(lp->optnam, cp))
			{
				*(lp->optadd) = 1;
				cp += strlen(lp->optnam);
				if (letter(*cp))
				error("Misspelled global option");
				while (*cp==' ')cp++;
				t=nb;
				if ( *cp == '(')
				while ((ci= *++cp) != ')')
				*t++ = ci;
				else cp--;
				*t++ = 0; *t=0;
				if (lp->optadd == &tab)
				{
					if (nb[0])
					*(lp->optadd) = nb[0];
				}
				if (lp->optadd == &linsize)
				printf(".nr %d %s\n", LSIZE, nb);
				if (lp->optadd == &delim1)
				{
					delim1 = nb[0];
					delim2 = nb[1];
				}
				found=1;
				break;
			}
		}
		if (!found)
		error("Illegal option");
	}
	cp++;
	backrest(cp);
}

backrest(char *cp)
{
	char *s;

	for(s = cp; *s; s++)
		;
	un1getc('\n');
	while (s > cp)
		un1getc(*--s);
}
