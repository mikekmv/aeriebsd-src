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
static char sccsid[] = "@(#)t8.c	4.5 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t9.c	4.4 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)te.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tg.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)ti.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tm.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)ts.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tt.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tv.c	4.4 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

/* t8.c: write out one line of output table */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tbl.h"

#define realsplit ((ct=='a'||ct=='n') && table[nl][c].rcol)

int watchout;
int once;
int topat[MAXCOL];

/* i is line number for deciding format */
/* nl is line number for finding data   usually identical */
void
putline(int i, int nl)
{
	int c, lf, ct, form, lwid, vspf, ip = -1, cmidx = 0, exvspen, vforml;
	int vct, chfont;
	char *s, *size, *fn;

	watchout = vspf = exvspen = 0;
	if (i == 0)
		once=0;
	if (i == 0 && (allflg || boxflg || dboxflg))
		fullwide(0, dboxflg? '=' : '-');
	if (instead[nl] == 0 && fullbot[nl] == 0)
		for(c = 0; c < ncol; c++) {
			s = table[nl][c].col;
			if (s == 0)
				continue;
			if (vspen(s)) {
				for(ip=nl; ip<nlin; ip=next(ip))
					if (!vspen(s=table[ip][c].col))
						break;
				if (tx(s))
					fprintf(tabout, ".ne \\n(%c|u+\\n(.Vu\n", s);
				continue;
			}
			if (point(s))
				continue;
			fprintf(tabout, ".ne \\n(%c|u+\\n(.Vu\n", s);
			watchout = 1;
		}
	if (linestop[nl])
		fprintf(tabout, ".mk #%c\n", linestop[nl]+'a'-1);
	lf = prev(nl);
	if (instead[nl]) {
		puts(instead[nl]);
		return;
	}
	if (fullbot[nl]) {
		switch (ct=fullbot[nl]) {
		case '=':
		case '-':
			fullwide(nl,ct);
		}
		return;
	}
	for(c=0; c<ncol; c++) {
		if (instead[nl]==0 && fullbot[nl]==0)
			if (vspen(table[nl][c].col))
				vspf=1;
		if (lf>=0)
			if (vspen(table[lf][c].col))
				vspf=1;
	}
	if (vspf) {
		fprintf(tabout, ".nr #^ \\n(\\*(#du\n");
		fprintf(tabout, ".nr #- \\n(#^\n"); /* current line position relative to bottom */
	}
	vspf=0;
	chfont=0;
	for(c=0; c<ncol; c++) {
		s = table[nl][c].col;
		if (s==0)
			continue;
		chfont |= (int)(font[stynum[nl]][c]);
		if (point(s))
			continue;
		lf=prev(nl);
		if (lf>=0 && vspen(table[lf][c].col))
			fprintf(tabout, ".if (\\n(%c|+\\n(^%c-1v)>\\n(#- .nr #- +(\\n(%c|+\\n(^%c-\\n(#--1v)\n",s,'a'+c,s,'a'+c);
		else
			fprintf(tabout, ".if (\\n(%c|+\\n(#^-1v)>\\n(#- .nr #- +(\\n(%c|+\\n(#^-\\n(#--1v)\n",s,s);
	}
	if (allflg && once>0 )
		fullwide(i,'-');
	once=1;
	runtabs(i, nl);
	if (allh(i) && !pr1403) {
		fprintf(tabout, ".nr %d \\n(.v\n", SVS);
		fprintf(tabout, ".vs \\n(.vu-\\n(.sp\n");
	}
	if (chfont)
		fprintf(tabout, ".nr %2d \\n(.f\n", S1);
	fprintf(tabout, ".nr 35 1m\n");
	fprintf(tabout, "\\&");
	vct = 0;
	for(c=0; c<ncol; c++) {
		if (watchout==0 && i+1<nlin && (lf=left(i,c, &lwid))>=0) {
			tohcol(c);
			drawvert(lf, i, c, lwid);
			vct += 2;
		}
		if (rightl && c+1==ncol)
			continue;
		vforml=i;
		for(lf=prev(nl); lf>=0 && vspen(table[lf][c].col); lf=prev(lf))
			vforml= lf;
		form= ctype(vforml,c);
		if (form != 's') {
			ct = c+CLEFT;
			if (form=='a')
				ct = c+CMID;
			if (form=='n' && table[nl][c].rcol && lused[c]==0)
				ct= c+CMID;
			fprintf(tabout, "\\h'|\\n(%du'", ct);
		}
		s= table[nl][c].col;
		fn = font[stynum[vforml]][c];
		size = csize[stynum[vforml]][c];
		if (*size==0)size=0;
		switch(ct=ctype(vforml, c)) {
		case 'n':
		case 'a':
			if (table[nl][c].rcol) {
				if (lused[c]) /*Zero field width*/ {
					ip = prev(nl);
					if (ip>=0 && vspen(table[ip][c].col)) {
						if (exvspen==0) {
							fprintf(tabout, "\\v'-(\\n(\\*(#du-\\n(^%cu", c+'a');
							if (cmidx)
								fprintf(tabout, "-((\\n(#-u-\\n(^%cu)/2u)", c+'a');
							vct++;
							fprintf(tabout, "'");
							exvspen=1;
						}
					}
					fprintf(tabout, "%c%c",F1,F2);
					puttext(s, fn, size);
					fprintf(tabout, "%c",F1);
				}
				s= table[nl][c].rcol;
				form=1;
				break;
			}
		case 'c':
			form=3; break;
		case 'r':
			form=2; break;
		case 'l':
			form=1; break;
		case '-':
		case '=':
			if (real(table[nl][c].col))
				fprintf(stderr,"%s: line %d: Data ignored on table line %d\n", ifile, iline-1, i+1);
			makeline(i,c,ct);
			continue;
			default:
			continue;
		}
		if (realsplit ? rused[c]: used[c]) { /*Zero field width*/
			/* form: 1 left, 2 right, 3 center adjust */
			if (ifline(s)) {
				makeline(i,c,ifline(s));
				continue;
			}
			if (filler(s)) {
				printf("\\l'|\\n(%du\\&%s'", c+CRIGHT, s+2);
				continue;
			}
			ip = prev(nl);
			cmidx = ctop[stynum[nl]][c]==0;
			if (ip>=0 && vspen(table[ip][c].col)) {
				if (exvspen==0) {
					fprintf(tabout, "\\v'-(\\n(\\*(#du-\\n(^%cu", c+'a');
					if (cmidx)
						fprintf(tabout, "-((\\n(#-u-\\n(^%cu)/2u)", c+'a');
					vct++;
					fprintf(tabout, "'");
				}
			}
			fprintf(tabout, "%c", F1);
			if (form!= 1)
				fprintf(tabout, "%c", F2);
			if (vspen(s))
				vspf=1;
			else
				puttext(s, fn, size);
			if (form !=2)
				fprintf(tabout, "%c", F2);
			fprintf(tabout, "%c", F1);
		}
		if (ip >= 0) {
			if (vspen(table[ip][c].col)) {
				exvspen = (c + 1 < ncol) &&
				    vspen(table[ip][c+1].col) &&
				    (topat[c] == topat[c + 1]) &&
				    (cmidx == !ctop[stynum[nl]][c + 1]) &&
				    (left(i, c + 1, &lwid) < 0);
				if (exvspen == 0) {
					fprintf(tabout,
					    "\\v'(\\n(\\*(#du-\\n(^%cu", c+'a');
					if (cmidx)
						fprintf(tabout,
						    "-((\\n(#-u-\\n(^%cu)/2u)",
						    c + 'a');
					vct++;
					fprintf(tabout, "'");
				}
			} else
				exvspen = 0;
		}
		/* if lines need to be split for gcos here is the place for a backslash */
		if (vct > 7 && c < ncol) {
			fprintf(tabout, "\n.sp-1\n\\&");
			vct = 0;
		}
	}
	fprintf(tabout, "\n");
	if (allh(i) && !pr1403)
		fprintf(tabout, ".vs \\n(%du\n", SVS);
	if (watchout)
		funnies(i,nl);
	if (vspf) {
		for(c=0; c<ncol; c++)
		if (vspen(table[nl][c].col) && (nl==0 || (lf=prev(nl))<0 || !vspen(table[lf][c].col)))
		{
			fprintf(tabout, ".nr ^%c \\n(#^u\n", 'a'+c);
			topat[c]=nl;
		}
	}
}

void
puttext(const char *s, const char *fn, const char *size)
{
	if (point(s)) {
		putfont(fn);
		putsize(size);
		fprintf(tabout, "%s",s);
		if (*fn > 0)
			fprintf(tabout, "\\f\\n(%2d", S1);
		if (size!=0)
			putsize("0");
	}
}

void
funnies(int stl, int lin)
{
	/* write out funny diverted things */
	int c, pl, lwid, dv, lf, ct;
	const char *s;
	char *fn;

	fprintf(tabout, ".mk ##\n"); /* remember current vertical position */
	fprintf(tabout, ".nr %d \\n(##\n", S1); /* bottom position */
	for(c = 0; c < ncol; c++) {
		s = table[lin][c].col;
		if (point(s))
			continue;
		if (s == NULL)
			continue;
		fprintf(tabout, ".sp |\\n(##u-1v\n");
		fprintf(tabout, ".nr %d ", SIND);
		for (ct = 0, pl = stl; pl >= 0 && !isalpha(ct = ctype(pl,c));
		    pl = prev(pl))
			;
		switch (ct) {
		case 'n':
		case 'c':
			fprintf(tabout, "(\\n(%du+\\n(%du-\\n(%c-u)/2u\n", 
			    c + CLEFT, c - 1 + ctspan(lin, c) + CRIGHT, s);
			break;
		case 'l':
			fprintf(tabout, "\\n(%du\n",c+CLEFT);
			break;
		case 'a':
			fprintf(tabout, "\\n(%du\n",c+CMID);
			break;
		case 'r':
			fprintf(tabout, "\\n(%du-\\n(%c-u\n", c+CRIGHT, s);
			break;
		}
		fprintf(tabout, ".in +\\n(%du\n", SIND);
		fn = font[stynum[stl]][c];
		putfont(fn);
		pl = prev(stl);
		if (stl > 0 && pl >= 0 && vspen(table[pl][c].col)) {
			fprintf(tabout, ".sp |\\n(^%cu\n", 'a'+c);
			if (ctop[stynum[stl]][c]==0) {
				fprintf(tabout, ".nr %d \\n(#-u-\\n(^%c-\\n(%c|+1v\n",TMP, 'a'+c, s);
				fprintf(tabout, ".if \\n(%d>0 .sp \\n(%du/2u\n", TMP, TMP);
			}
		}
		fprintf(tabout, ".%c+\n",s);
		fprintf(tabout, ".in -\\n(%du\n", SIND);
		if (*fn>0)
			putfont("P");
		fprintf(tabout, ".mk %d\n", S2);
		fprintf(tabout, ".if \\n(%d>\\n(%d .nr %d \\n(%d\n", S2, S1, S1, S2);
	}
	fprintf(tabout, ".sp |\\n(%du\n", S1);
	for(c=dv=0; c<ncol; c++) {
		if (stl+1< nlin && (lf=left(stl,c,&lwid))>=0) {
			if (dv++ == 0)
				fprintf(tabout, ".sp -1\n");
			tohcol(c);
			dv++;
			drawvert(lf, stl, c, lwid);
		}
	}
	if (dv)
		fprintf(tabout,"\n");
}

void
putfont(const char *fn)
{
	if (fn && *fn)
		fprintf(tabout, fn[1] ? "\\f(%.2s" : "\\f%.2s", fn);
}

void
putsize(const char *s)
{
	if (s && *s)
		fprintf(tabout, "\\s%s",s);
}

int
point(const char *s)
{
	return ((long)s >= 128 || (long)s < 0);
}

/* t9.c: write lines for tables over 200 lines */

static int useln;
void
yetmore(void)
{
	for(useln = 0; useln < MAXLIN && table[useln] == 0; useln++);
		if (useln>=MAXLIN)
			error("Weird.  No data in table.");
	table[0] = table[useln];
	for(useln = nlin - 1; useln >= 0 &&
	    (fullbot[useln] || instead[useln]); useln--)
		;
	if (useln<0)
		error("Weird.  No real lines in table.");
	domore(leftover);
	while (gets1(cstore = cspace) && domore(cstore))
		;
	last = cstore;
}

int
domore(char *dataln)
{
	int icol, ch;

	if (prefix(".TE", dataln))
		return(0);
	if (dataln[0] == '.' && !isdigit(dataln[1])) {
		puts(dataln);
		return(1);
	}
	instead[0] = NULL;
	fullbot[0] = 0;
	if (dataln[1]==0)
		switch(dataln[0]) {
		case '_': fullbot[0]= '-'; putline(useln,0); return(1);
		case '=': fullbot[0]= '='; putline(useln, 0); return(1);
		}
	for (icol = 0; icol < ncol; icol++) {
		table[0][icol].col = dataln;
		table[0][icol].rcol=0;
		for(; (ch= *dataln) != '\0' && ch != tab; dataln++)
			;
		*dataln++ = '\0';
		switch(ctype(useln,icol)) {
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
		if (ch == '\0')
			break;
	}
	while (++icol <ncol)
		table[0][icol].col = "";
	putline(useln,0);
	return(1);
}

/* te.c: error message control, input line count */

void
error(const char *s)
{
	fprintf(stderr, "\n%s: line %d: %s\n", ifile, iline, s);
	fprintf(stderr, "tbl quits\n");
	exit(1);
}

char *
gets1(char *s)
{
	char *p;
	int nbl = 0;
	iline++;
	p=fgets(s,BUFSIZ,tabin);
	while (p==0) {
		if (swapin()==0)
			return(0);
		p = fgets(s,BUFSIZ,tabin);
	}

	while (*s)
		s++;
	s--;
	if (*s == '\n')
		*s-- =0;
	for(nbl=0; *s == '\\' && s>p; s--)
		nbl++;
	if (linstart && nbl % 2) /* fold escaped nl if in table */
		gets1(s+1);

	return(p);
}

# define BACKMAX 500
char backup[BACKMAX];
char *backp = backup;
void
un1getc(int c)
{
	if (c=='\n')
		iline--;
	*backp++ = c;
	if (backp >= backup+BACKMAX)
		error("too much backup");
}

int
get1char(void)
{
	int c;

	if (backp > backup)
		c = *--backp;
	else
		c = getc(tabin);
	if (c == EOF)  {
		if (swapin() == 0)
			error("unexpected EOF");
		c = getc(tabin);
	}
	if (c == '\n')
		iline++;
	return(c);
}

/* tg.c: process included text blocks */

/* get a section of text */
int
gettext(char *sp, int ilin, int icol, const char *fn, const char *sz)
{
	char line[256];
	int oname;
	char *vs;

	if (texname==0)
		error("Too many text block diversions");

	if (textflg==0) {
		/* remember old line length */
		fprintf(tabout, ".nr %d \\n(.lu\n", SL);
		textflg = 1;
	}
	fprintf(tabout, ".eo\n");
	fprintf(tabout, ".am %02d\n", icol+80);
	fprintf(tabout, ".br\n");
	fprintf(tabout, ".di %c+\n", texname);
	rstofill();
	if (fn && *fn)
		fprintf(tabout, ".nr %d \\n(.f\n.ft %s\n", S1, fn);
	fprintf(tabout, ".ft \\n(.f\n"); /* protect font */
	vs = vsize[stynum[ilin]][icol];
	if ((sz && *sz) || (vs && *vs)) {
		fprintf(tabout, ".nr %d \\n(.v\n", S2);
		if (vs==0 || *vs==0)
			vs = "\\n(.s+2";
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
	while (gets1(line)) {
		if (line[0]=='T' && line[1]=='}' && line[2]== tab)
			break;
		if (match("T}", line))
			break;
		fprintf(tabout, "%s\n", line);
	}
	if (fn && *fn)
		fprintf(tabout, ".ft \\n(%d\n", S1);
	if (sz && *sz)
		fprintf(tabout, ".br\n.ps\n.vs\n");
	fprintf(tabout, ".br\n");
	fprintf(tabout, ".di\n");
	fprintf(tabout, ".nr %c| \\n(dn\n", texname);
	fprintf(tabout, ".nr %c- \\n(dl\n", texname);
	fprintf(tabout, "..\n");
	fprintf(tabout, ".ec \\\n");
	/* copy remainder of line */
	if (line[2])
		strcpy(sp, line+3);
	else
		*sp = 0;
	oname=texname;
	texname = texstr[++texct];
	return(oname);
}

void
untext(void)
{
	rstofill();
	fprintf(tabout, ".nf\n");
	fprintf(tabout, ".ll \\n(%du\n", SL);
}

/* ti.c: classify line intersections */

/* determine local environment for intersections */
int
interv(int i, int c)
{
	int ku, kl;

	if (c >= ncol || c == 0) {
		if (dboxflg) {
			if (i == 0)
				return (BOT);
			if (i >= nlin)
				return	(TOP);
			return	(THRU);
		}
		if (c >= ncol)
			return (0);
	}
	ku = i > 0 ? lefdata(i - 1,c) : 0;
	if (i+1 >= nlin)
		kl = 0;
	else
		kl = lefdata(allh(i) ? i + 1 : i, c);
	if (ku == 2 && kl == 2)
		return (THRU);
	if (ku == 2)
		return (TOP);
	if (kl == BOT)
		return (2);
	return (0);
}

int
interh(int i, int c)
{
	int kl, kr;
	if (fullbot[i] == '=' || (dboxflg && (i == 0 || i >= nlin-1))) {
		if (c == ncol)
			return (LEFT);
		if (c == 0)
			return (RIGHT);
		return (THRU);
	}
	if (i >= nlin)
		return(0);
	kl = c > 0? thish(i, c-1) : 0;
	if (kl <= 1 && i > 0 && allh(up1(i)))
		kl = c > 0? thish(up1(i), c - 1) : 0;
	kr = thish(i, c);
	if (kr <= 1 && i > 0 && allh(up1(i)))
		kr = c > 0? thish(up1(i), c) : 0;
	if (kl == '=' && kr == '=')
		return (THRU);
	if (kl == '=')
		return (LEFT);
	if (kr == '=')
		return (RIGHT);
	return (0);
}

int
up1(int i)
{
	i--;
	while (instead[i] && i > 0)
		i--;
	return (i);
}

/* tm.c: split numerical fields */

/* make two numerical fields */
char *
maknew(char *str)
{
	char *p, *q, *ba, *dpoint;
	int c;

	p = str;
	for (ba = 0; (c = *str); str++)
		if (c == '\\' && *(str + 1) == '&')
			ba = str;
	str = p;
	if (ba == 0) {
		for (dpoint = 0; *str; str++) {
			if (*str == '.' && !ineqn(str,p) &&
			    ((str > p && isdigit(*(str-1))) ||
			    isdigit(*(str+1))))
				dpoint = str;
		}
		if (!dpoint)
			for(; str > p; str--) {
				if (isdigit(str[-1]) && !ineqn(str, p))
					break;
			}
		if (!dpoint && p == str) /* not numerical, don't split */
			return (0);
		if (dpoint)
			str=dpoint;
	}
	else
	str = ba;
	p =str;
	if (exstore ==0 || exstore >exlim)
	{
		exstore = chspace();
		exlim= exstore+MAXCHS;
	}
	q = exstore;
	while ((*exstore++ = *str++))
		;
	*p = 0;
	return(q);
}

/* true if s is in a eqn within p */
int
ineqn (const char *s, const char *p)
{
	int ineq = 0, c;

	while ((c = *p)) {
		if (s == p)
			return (ineq);
		p++;
		if ((ineq == 0) && (c == delim1))
			ineq = 1;
		else if ((ineq == 1) && (c == delim2))
			ineq = 0;
	}
	return (0);
}

/* ts.c: minor string processing subroutines */

int
match(const char *s1, const char *s2)
{
	while (*s1 == *s2)
		if (*s1++ == '\0')
			return(1);
		else
			s2++;
	return(0);
}

int
prefix(const char *small, const char *big)
{
	int c;

	while ((c = *small++) == *big++)
		if (c == 0)
			return (1);
	return (c == 0);
}

/* tt.c: subroutines for drawing horizontal lines */

int
ctype(int il, int ic)
{
	if (instead[il])
		return (0);
	if (fullbot[il])
		return (0);
	il = stynum[il];
	return (style[il][ic]);
}

int
fspan(int i, int c)
{
	c++;
	return (c < ncol && ctype(i, c) == 's');
}

int
lspan(int i, int c)
{
	int k;

	if (ctype(i, c) != 's')
		return(0);
	c++;
	if (c < ncol && ctype(i, c)== 's')
		return (0);
	for(k = 0; ctype(i, --c) == 's'; k++);
		return (k);
}

int
ctspan(int i, int c)
{
	int k;

	for(c++, k = 1; c < ncol && ctype(i, c) == 's'; k++)
		c++;
	return (k);
}

void
tohcol(int ic)
{
	if (ic == 0)
		fprintf(tabout, "\\h'|0'");
	else
		fprintf(tabout, "\\h'(|\\n(%du+|\\n(%du)/2u'",
		    ic + CLEFT, ic + CRIGHT - 1);
}

int
allh(int i)
{
	/* return true if every element in line i is horizontal */
	/* also at least one must be horizontl */
	int c, one, k;

	if (fullbot[i])
		return (1);

	for(one = c = 0; c < ncol; c++) {
		k = thish(i, c);
		if (k == 0)
			return(0);
		if (k == 1)
			continue;
		one = 1;
	}
	return (one);
}

int
thish(int i, int c)
{
	int t;
	char *s;
	struct colstr *pc;

	if (c < 0)
		return(0);
	if (i < 0)
		return(0);
	t = ctype(i,c);
	if (t=='_' || t == '-')
		return('-');
	if (t=='=')
		return('=');
	if (t=='^')
		return(1);
	if (fullbot[i])
		return(fullbot[i]);
	if (t=='s')
		return(thish(i,c-1));
	if (t==0)
		return(1);
	pc = &table[i][c];
	s = (t == 'a' ? pc->rcol : pc->col);
	if (s==0 || (point(s) && *s==0))
		return(1);
	if (vspen(s))
		return(1);
	if ((t = barent(s)))
		return (t);
	return (0);
}

/* tv.c: draw vertical lines */

void
drawvert(int start, int end, int c, int lwid)
{
	char *exb=0, *ext=0;
	int tp=0, sl, ln, pos, epb, ept, vm;
	end++;
	vm='v';
	/* note: nr 35 has value of 1m outside of linesize */
	while (instead[end])
		end++;
	for(ln=0; ln < lwid; ln++) {
		epb=ept=0;
		pos = 2*ln-lwid+1;
		if (pos!=tp) fprintf(tabout, "\\h'%dp'", pos-tp);
		tp = pos;
		if (end<nlin) {
			if (fullbot[end] || (!instead[end] && allh(end)))
				epb=2;
			else
				switch (midbar(end,c)) {
				case '-':
					exb = "1v-.5m";
					break;
				case '=':
					exb = "1v-.5m";
					epb = 1;
					break;
				}
		}
		if (lwid>1)
			switch(interh(end, c)) {
			case THRU: epb -= 1; break;
			case RIGHT: epb += (ln==0 ? 1 : -1); break;
			case LEFT: epb += (ln==1 ? 1 : -1); break;
			}
		if (lwid==1)
			switch(interh(end,c)) {
			case THRU: epb -= 1; break;
			case RIGHT: case LEFT: epb += 1; break;
			}
		if (start>0) {
			sl = start-1;
			while (sl>=0 && instead[sl])
				sl--;
			if (sl>=0 && (fullbot[sl] || allh(sl)))
				ept=0;
			else
				if (sl>=0)
				switch(midbar(sl,c))
				{
				case '-': ext = ".5m"; break;
				case '=': ext= ".5m"; ept = -1; break;
				default:  vm = 'm'; break;
				}
			else
				ept = -4;
		}
		else if (start==0 && allh(0)) {
			ept=0;
			vm = 'm';
		}
		if (lwid>1)
			switch(interh(start,c))
			{
			case THRU: ept += 1; break;
			case LEFT: ept += (ln==0 ? 1 : -1); break;
			case RIGHT: ept += (ln==1 ? 1 : -1); break;
			}
		else if (lwid==1)
			switch(interh(start,c))
			{
			case THRU: ept += 1; break;
			case LEFT: case RIGHT: ept -= 1; break;
			}
		if (exb)
			fprintf(tabout, "\\v'%s'", exb);
		if (epb)
			fprintf(tabout, "\\v'%dp'", epb);
		fprintf(tabout, "\\s\\n(%d",LSIZE);
		if (linsize)
			fprintf(tabout, "\\v'-\\n(%dp/6u'", LSIZE);
		fprintf(tabout, "\\h'-\\n(#~u'"); /* adjustment for T450 nroff boxes */
		fprintf(tabout, "\\L'|\\n(#%cu-%s", linestop[start]+'a'-1, vm=='v'? "1v" : "\\n(35u");
		if (ext)
			fprintf(tabout, "-(%s)",ext);
		if (exb)
			fprintf(tabout, "-(%s)", exb);
		pos = ept-epb;
		if (pos)
			fprintf(tabout, "%s%dp", pos>=0? "+" : "", pos);
		/* the string #d is either "nl" or ".d" depending
		 on diversions; on GCOS not the same */
		fprintf(tabout, "'\\s0\\v'\\n(\\*(#du-\\n(#%cu+%s", linestop[start]+'a'-1,vm=='v' ? "1v" : "\\n(35u");
		if (ext)
			fprintf(tabout, "+%s",ext);
		if (ept)
			fprintf(tabout, "%s%dp", (-ept)>0 ? "+" : "", (-ept));
		fprintf(tabout, "'");
		if (linsize)
			fprintf(tabout, "\\v'\\n(%dp/6u'", LSIZE);
	}
}

int
midbar(int i, int c)
{
	int k = midbcol(i,c);

	if (k == 0 && c > 0)
		k = midbcol(i, c - 1);

	return (k);
}

int
midbcol(int i, int c)
{
	int ct;

	while ((ct = ctype(i,c)) == 's')
		c--;
	if (ct == '-' || ct == '=')
		return (ct);
	if ((ct = barent(table[i][c].col)))
		return (ct);
	return (0);
}

int
barent(const char *s)
{
	if (s == 0)
		return (1);
	if (!point(s))
		return (1);
	if (s[0] == '\\')
		s++;
	if (s[0] && s[1])
		return (0);
	switch(s[0]) {
	case '_': return ('-');
	case '=': return ('=');
	}
	return (0);
}

/*
 * returns -1 if no line to left
 * returns number of line where it starts
 * stores into lwid the kind of line
 */
int
left(int i, int c, int *lwidp)
{
	int kind, li, lj;

	*lwidp=0;
	kind = lefdata(i, c);
	if (kind == 0)
		return(-1);
	if (i+1 < nlin)
		if (lefdata(next(i), c)== kind)
			return(-1);
	li = -1;
	while (i >= 0 && lefdata(i, c) == kind)
		i = prev(li = i);
	if (i == -1)
		li = 0;
	*lwidp = kind;
	for(lj= i+1; lj<li; lj++)
		if (instead[lj] && strcmp(instead[lj], ".TH")==0)
			return(li);
	for(i= i+1; i<li; i++)
		if (fullbot[i])
			li=i;
	return(li);
}

int
lefdata(int i, int c)
{
	int ck;

	if (i >= nlin)
		i = nlin - 1;
	if (ctype(i,c) == 's') {
		for(ck=c; ctype(i,ck)=='s'; ck--);
			if (thish(i,ck)==0)
				return(0);
	}
	i =stynum[i];
	i = lefline[i][c];

	if (i > 0)
		return(i);
	if (dboxflg && c==0)
		return(2);
	if (allflg)
		return(1);
	if (boxflg && c==0)
		return(1);
	return(0);
}

int
next(int i)
{
	while (i + 1 < nlin) {
		i++;
		if (!fullbot[i] && !instead[i])
			break;
	}

	return(i);
}

int
prev(int i)
{
	while (--i >= 0 && (fullbot[i] || instead[i]))
		;
	return (i);
}

int spcount = 0;
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
alocv(int n)
{
	int *tp, *q;
	if (tpcount < 0 || thisvec + n > tpvecs[tpcount] + MAXCHS) {
		tpcount++;
		if (tpvecs[tpcount] == 0)
			tpvecs[tpcount] = calloc(MAXCHS, 1);
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

void
release(void)
{
	extern char *exstore;
	/* give back unwanted space in some vectors */
	spcount=0;
	tpcount= -1;
	exstore=0;
}
