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
static char sccsid[] = "@(#)lookup.c	4.3 (Berkeley) 4/17/91";
#else
static const char rcsid[] = "$ABSD: lookup.c,v 1.3 2009/10/24 15:30:57 mickey Exp $";
#endif
#endif /* not lint */

#include "e.h"
#include "y.tab.h"

#define	TBLSIZE	100

tbl	*keytbl[TBLSIZE];	/* key words */
tbl	*restbl[TBLSIZE];	/* reserved words */
tbl	*deftbl[TBLSIZE];	/* user-defined names */

const struct {
	const char key[12];
	long	keyval;
} keyword[] = {
	"sub",		SUB,
	"sup",		SUP,
	".EN",		EOF,
	"from",		FROM,
	"to",		TO,
	"sum",		SUM,
	"hat",		HAT,
	"vec",		VEC,
	"dyad",		DYAD,
	"dot",		DOT,
	"dotdot",	DOTDOT,
	"bar",		BAR,
	"tilde",	TILDE,
	"under",	UNDER,
	"prod",		PROD,
	"int",		INT,
	"integral",	INT,
	"union",	UNION,
	"inter",	INTER,
	"pile",		PILE,
	"lpile",	LPILE,
	"cpile",	CPILE,
	"rpile",	RPILE,
	"over",		OVER,
	"sqrt",		SQRT,
	"above",	ABOVE,
	"size",		SIZE,
	"font",		FONT,
	"fat",		FAT,
	"roman",	ROMAN,
	"italic",	ITALIC,
	"bold",		BOLD,
	"left",		LEFT,
	"right",	RIGHT,
	"delim",	DELIM,
	"define",	DEFINE,
	"tdefine",	TDEFINE,
	"ndefine",	NDEFINE,
	"gsize",	GSIZE,
	".gsize",	GSIZE,
	"gfont",	GFONT,
	"include",	INCLUDE,
	"up",		UP,
	"down",		DOWN,
	"fwd",		FWD,
	"back",		BACK,
	"mark",		MARK,
	"lineup",	LINEUP,
	"matrix",	MATRIX,
	"col",		COL,
	"lcol",		LCOL,
	"ccol",		CCOL,
	"rcol",		RCOL,
	"",	0
};

const struct resword {
	const char *res;
	const char *resval;
} resword[] = {
	">=",		"\\(>=",
	"<=",		"\\(<=",
	"==",		"\\(==",
	"!=",		"\\(!=",
	"+-",		"\\(+-",
	"->",		"\\(->",
	"<-",		"\\(<-",
	"inf",		"\\(if",
	"infinity",	"\\(if",
	"partial",	"\\(pd",
	"half",		"\\f1\\(12\\fP",
	"prime",	"\\f1\\(fm\\fP",
	"dollar",	"\\f1$\\fP",
	"nothing",	"",
	"times",	"\\(mu",
	"del",		"\\(gr",
	"grad",		"\\(gr",
	"alpha",	"\\(*a",
	"beta",		"\\(*b",
	"gamma",	"\\(*g",
	"GAMMA",	"\\(*G",
	"delta",	"\\(*d",
	"DELTA",	"\\(*D",
	"epsilon",	"\\(*e",
	"EPSILON",	"\\f1E\\fP",
	"omega",	"\\(*w",
	"OMEGA",	"\\(*W",
	"lambda",	"\\(*l",
	"LAMBDA",	"\\(*L",
	"mu",		"\\(*m",
	"nu",		"\\(*n",
	"theta",	"\\(*h",
	"THETA",	"\\(*H",
	"phi",		"\\(*f",
	"PHI",		"\\(*F",
	"pi",		"\\(*p",
	"PI",		"\\(*P",
	"sigma",	"\\(*s",
	"SIGMA",	"\\(*S",
	"xi",		"\\(*c",
	"XI",		"\\(*C",
	"zeta",		"\\(*z",
	"iota",		"\\(*i",
	"eta",		"\\(*y",
	"kappa",	"\\(*k",
	"rho",		"\\(*r",
	"tau",		"\\(*t",
	"omicron",	"\\(*o",
	"upsilon",	"\\(*u",
	"UPSILON",	"\\(*U",
	"psi",		"\\(*q",
	"PSI",		"\\(*Q",
	"chi",		"\\(*x",
	"and",		"\\f1and\\fP",
	"for",		"\\f1for\\fP",
	"if",		"\\f1if\\fP",
	"Re",		"\\f1Re\\fP",
	"Im",		"\\f1Im\\fP",
	"sin",		"\\f1sin\\fP",
	"cos",		"\\f1cos\\fP",
	"tan",		"\\f1tan\\fP",
	"sec",		"\\f1sec\\fP",
	"csc",		"\\f1csc\\fP",
	"arc",		"\\f1arc\\fP",
	"asin",		"\\f1asin\\fP",
	"acos",		"\\f1acos\\fP",
	"atan",		"\\f1atan\\fP",
	"asec",		"\\f1asec\\fP",
	"acsc",		"\\f1acsc\\fP",
	"sinh",		"\\f1sinh\\fP",
	"coth",		"\\f1coth\\fP",
	"tanh",		"\\f1tanh\\fP",
	"cosh",		"\\f1cosh\\fP",
	"lim",		"\\f1lim\\fP",
	"log",		"\\f1log\\fP",
	"max",		"\\f1max\\fP",
	"min",		"\\f1min\\fP",
	"ln",		"\\f1ln\\fP",
	"exp",		"\\f1exp\\fP",
	"det",		"\\f1det\\fP",
}, neqn_resw[] = {
	"<<",		"<<",
	">>",		">>",
	"approx",	"~\b\\d~\\u",
	"cdot",		"\\v'-.5'.\\v'.5'",
	"...",		"...",
	",...,",	",...,",
}, eqn_resw[] = {
	"<<",		"<\\h'-.3m'<",
	">>",		">\\h'-.3m'>",
	"approx",	"\\v'-.2m'\\z\\(ap\\v'.25m'\\(ap\\v'-.05m'",
	"cdot",		"\\v'-.3m'.\\v'.3m'",
	"...",		"\\v'-.3m'\\ .\\ .\\ .\\ \\v'.3m'",
	",...,",	",\\ .\\ .\\ .\\ ,\\|",
};

/*
 * find name in tbl. if defn non-null, install
 */
tbl *
lookup(tbl **tblp, const char *name, const char *defn)
{
	int h;
	const char *s = name;
	tbl *p;

	for (h = 0; *s != '\0'; )
		h += *s++;
	h %= TBLSIZE;

	for (p = tblp[h]; p != NULL; p = p->next)
		if (strcmp(name, p->name) == 0) {	/* found it */
			if (defn != NULL)
				p->defn = defn;
			return(p);
		}
	/* didn't find it */
	if (defn == NULL)
		return(NULL);
	p = malloc(sizeof (tbl));
	if (p == NULL)
		error(FATAL, "out of space in lookup");
	p->name = name;
	p->defn = defn;
	p->next = tblp[h];
	tblp[h] = p;
	return(p);
}

void
init_tbl(void)	/* initialize all tables */
{
	const struct resword *r;
	int i, val;

	for (i = 0; keyword[i].key[0] != '\0'; i++) {
		val = keyword[i].keyval;
		if (neqn && val == NDEFINE)
			val = DEFINE;
		if (!neqn && val == TDEFINE)
			val = DEFINE;
		lookup(keytbl, keyword[i].key, (char *)keyword[i].keyval);
	}
	for (i = 0, r = resword;
	    i < sizeof resword / sizeof resword[0]; i++, r++)
		lookup(restbl, r->res, r->resval);
	for (i = 0, r = neqn? neqn_resw : eqn_resw;
	    i < sizeof eqn_resw / sizeof eqn_resw[0]; i++, r++)
		lookup(restbl, r->res, r->resval);
}
