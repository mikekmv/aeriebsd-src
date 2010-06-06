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
static char sccsid[] = "@(#)t0.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t1.c	4.5 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t2.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t3.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t4.c	4.4 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t5.c	4.4 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t6.c	4.5 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)t7.c	4.5 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tb.c	4.6 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tc.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tf.c	4.3 (Berkeley) 4/18/91";
static char sccsid[] = "@(#)tu.c	4.4 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD: tbl.c,v 1.2 2010/06/05 14:19:31 mickey Exp $";
#endif
#endif /* not lint */

#include <sys/param.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "pathnames.h"

#include "tbl.h"

/* t0.c: storage allocation */

int expflg = 0;
int ctrflg = 0;
int boxflg = 0;
int dboxflg = 0;
int tab = '\t';
int linsize;
int pr1403;
int delim1, delim2;
int nokeep;
int evenup[MAXCOL], evenflg;
int F1 = 0;
int F2 = 0;
int allflg = 0;
char *leftover;
int textflg = 0;
int left1flg = 0;
int rightl = 0;
char *cstore, *cspace;
char *last;
struct colstr *table[MAXLIN];
int style[MAXHEAD][MAXCOL];
int ctop[MAXHEAD][MAXCOL];
char font[MAXHEAD][MAXCOL][2];
char csize[MAXHEAD][MAXCOL][4];
char vsize[MAXHEAD][MAXCOL][4];
int lefline[MAXHEAD][MAXCOL];
char cll[MAXCOL][CLLEN];
/*char *rpt[MAXHEAD][MAXCOL];*/
/*char rpttx[MAXRPT];*/
int stynum[MAXLIN + 1];
int nslin, nclin;
int sep[MAXCOL];
int fullbot[MAXLIN];
char *instead[MAXLIN];
int used[MAXCOL], lused[MAXCOL], rused[MAXCOL];
int linestop[MAXLIN];
int nlin, ncol;
int iline = 1;
char *ifile = "Input";
int texname = 'a';
int texct = 0;
const char texstr[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWYXZ0123456789";
int linstart;
char *exstore, *exlim;

int *alocv(int);
void badsig(int);

/* t1.c: main control and input switching */

int sargc;
char **sargv;

int
main(int argc, char *argv[])
{
	char line[BUFSIZ];
	int i;

	signal(SIGPIPE, badsig);

	sargc = argc;
	sargv = argv;
	sargc--; sargv++;
	if (sargc > 0)
		swapin();
	while (gets1(line)) {
		puts(line);
		if (prefix(".TS", line)) {
			puts(".if \\n+(b.=1 .nr d. \\n(.c-\\n(c.-1");
			linstart = iline;

			/*
			 * remembers various things:
			 *   fill mode, vs, ps in mac 35 (SF)
			 */
			printf(".de %d\n"
			    ".ps \\n(.s\n"
			    ".vs \\n(.vu\n"
			    ".in \\n(.iu\n"
			    ".if \\n(.u .fi\n"
			    ".if \\n(.j .ad\n"
			    ".if \\n(.j=0 .na\n"
			    "..\n"
			    ".nf\n"
			/* set obx offset if useful */
			    ".nr #~ 0\n"
			    ".if n .nr #~ 0.6n\n", SF);

			puts(".ds #d .d\n"
			    ".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl");
			puts(".fc");

			getcomm();
			getspec();
			gettbl();
			getstop();
			checkuse();
			choochar();
			maktab();
			runout();

			spcount = 0;
			tpcount = -1;
			exstore = 0;

			rstofill();

			for(i = 0; i < MAXHEAD; i++)
				if (linestop[i])
					printf(".nr #%c 0\n", 'a' + i);
			for(i = 0; i < texct; i++)
				printf(".rm %c+\n", texstr[i]);

			printf("%s\n.if \\n-(b.=0 .nr c. \\n(.c-\\n(d.-%d\n",
			    last, iline - linstart);
			linstart = 0;
		}
	}

	return(0);
}

int
swapin(void)
{
	while (sargc > 0 && **sargv=='-') { /* Mem fault if no test on sargc */
		if (sargc <= 0)
			return(0);
		if (match("-ms", *sargv)) {
			*sargv = _PATH_MACROS;
			break;
		}
		if (match("-mm", *sargv)) {
			*sargv = _PATH_PYMACS;
			break;
		}
		if (match("-TX", *sargv))
			pr1403 = 1;
		sargc--; sargv++;
	}
	if (sargc <= 0)
		return(0);

	if (!freopen(ifile = *sargv, "r", stdin))
		error("Can't open file");
	iline = 1;

	/* file names are all put into f. by the GCOS troff preprocessor */
	printf(".ds f. %s\n", ifile);

	sargc--;
	sargv++;
	return (1);
}

void
badsig(int sig)
{
	signal(SIGPIPE, SIG_IGN);
	_exit(0);
}

/* t3.c: interpret commands affecting whole table */

const struct optstr {
	const char *optnam;
	int *optadd;
} options[] = {
	{ "allbox", &allflg },
	{ "ALLBOX", &allflg },
	{ "box", &boxflg },
	{ "BOX", &boxflg },
	{ "center", &ctrflg },
	{ "CENTER", &ctrflg },
	{ "delim", &delim1 },
	{ "DELIM", &delim1 },
	{ "doublebox", &dboxflg },
	{ "DOUBLEBOX", &dboxflg },
	{ "expand", &expflg },
	{ "EXPAND", &expflg },
	{ "frame", &boxflg },
	{ "FRAME", &boxflg },
	{ "linesize", &linsize },
	{ "LINESIZE", &linsize },
	{ "nokeep", &nokeep },
	{ "NOKEEP", &nokeep },
	{ "tab", &tab },
	{ "TAB", &tab },
	{ NULL }
};

void
getcomm(void)
{
	char line[200], *cp, nb[25], *t;
	const struct optstr *lp;
	int c, ci, found;

	for(lp = options; lp->optnam; lp++)
		*(lp->optadd) = 0;
	texname = texstr[texct=0];
	tab = '\t';
	printf(".nr %d \\n(.s\n", LSIZE);
	gets1(line);
	/* see if this is a command line */
	if (index(line,';') == NULL) {
		backrest(line);
		return;
	}
	for(cp = line; (c = *cp) != ';'; cp++) {
		if (!isalpha(c))
			continue;
		found = 0;
		for (lp = options; lp->optadd; lp++) {
			if (prefix(lp->optnam, cp)) {
				*(lp->optadd) = 1;
				cp += strlen(lp->optnam);
				if (isalpha(*cp))
					error("Misspelled global option");
				while (*cp == ' ')
					cp++;
				t = nb;
				if (*cp == '(')
					while ((ci = *++cp) != ')')
						*t++ = ci;
				else
					cp--;
				*t++ = 0;
				*t = 0;
				if (lp->optadd == &tab && nb[0])
					*(lp->optadd) = nb[0];
				if (lp->optadd == &linsize)
					printf(".nr %d %s\n", LSIZE, nb);
				if (lp->optadd == &delim1) {
					delim1 = nb[0];
					delim2 = nb[1];
				}
				found = 1;
				break;
			}
		}
		if (!found)
		error("Illegal option");
	}
	cp++;
	backrest(cp);
}

void
backrest(const char *cp)
{
	const char *s;

	for(s = cp; *s; s++)
		;
	un1getc('\n');
	while (s > cp)
		un1getc(*--s);
}

/* t4.c: read table specification */

int oncol;

void
getspec(void)
{
	int icol, i;

	for (icol = 0; icol < MAXCOL; icol++) {
		sep[icol] = -1;
		evenup[icol] = 0;
		cll[icol][0] = 0;
		for(i = 0; i < MAXHEAD; i++) {
			csize[i][icol][0] = 0;
			vsize[i][icol][0] = 0;
			font[i][icol][0] = lefline[i][icol] = 0;
			ctop[i][icol] = 0;
			style[i][icol] = 'l';
		}
	}
	nclin = ncol = 0;
	oncol = 0;
	left1flg = rightl = 0;
	readspec();
	printf(".rm");
	for(i=0; i < ncol; i++)
		printf(" %02d", 80 + i);
	putchar('\n');
}

void
readspec(void)
{
	int icol, c, sawchar, stopc, i;
	char sn[10], *snp, *temp;

	sawchar = icol = 0;
	while ((c = get1char())) {
		switch(c) {
		default:
			if (c != tab)
				error("bad table specification character");
		case ' ': /* note this is also case tab */
			continue;
		case '\n':
			if(sawchar == 0)
				continue;
		case ',':
		case '.': /* end of table specification */
			ncol = MAX(ncol, icol);
			if (lefline[nclin][ncol]>0) {
				ncol++;
				rightl++;
			}
			if(sawchar)
				nclin++;
			if (nclin>=MAXHEAD)
				error("too many lines in specification");
			icol=0;
			if (ncol==0 || nclin==0)
				error("no specification");
			if (c== '.') {
				while ((c=get1char()) && c != '\n')
				if (c != ' ' && c != '\t')
				error("dot not last character on format line");
				/* fix up sep - default is 3 except at edge */
				for(icol=0; icol<ncol; icol++)
				if (sep[icol]<0)
				sep[icol] = icol+1<ncol ? 3 : 1;
				if (oncol == 0)
				oncol = ncol;
				else if (oncol +2 <ncol)
				error("tried to widen table in T&, not allowed");
				return;
			}
			sawchar=0;
			continue;
		case 'C': case 'S': case 'R': case 'N': case 'L': case 'A':
			c += ('a'-'A');
		case '_': if (c=='_') c= '-';
		case '=': case '-':
		case '^':
		case 'c': case 's': case 'n': case 'r': case 'l': case 'a':
			if (icol>=MAXCOL)
				error("too many columns in table");
			style[nclin][icol]=c;
			if (c== 's' && icol<=0)
				error("first column can not be S-type");
			if (c=='s' && style[nclin][icol-1] == 'a') {
				puts(".tm warning: can't span a-type cols, changed to l");
				style[nclin][icol-1] = 'l';
			}
			if (c=='s' && style[nclin][icol-1] == 'n') {
				puts(".tm warning: can't span n-type cols, changed to c");
				style[nclin][icol-1] = 'c';
			}
			icol++;
			if (c=='^' && nclin<=0)
				error("first row can not contain vertical span");
			sawchar=1;
			continue;
		case 'b': case 'i':
			c += 'A'-'a';
		case 'B': case 'I':
			if (icol==0) continue;
			snp=font[nclin][icol-1];
			snp[0]= (c=='I' ? '2' : '3');
			snp[1]=0;
			continue;
		case 't': case 'T':
			if (icol>0)
			ctop[nclin][icol-1] = 1;
			continue;
		case 'd': case 'D':
			if (icol>0)
			ctop[nclin][icol-1] = -1;
			continue;
		case 'f': case 'F':
			if (icol==0) continue;
			snp=font[nclin][icol-1];
			snp[0]=snp[1]=stopc=0;
			for(i=0; i<2; i++) {
				c = get1char();
				if (i==0 && c=='(') {
					stopc=')';
					c = get1char();
				}
				if (c==0) break;
				if (c==stopc) {stopc=0; break;}
				if (stopc==0) if (c==' ' || c== tab ) break;
				if (c=='\n') {un1getc(c); break;}
				snp[i] = c;
				if (c>= '0' && c<= '9') break;
			}
			if (stopc) if (get1char()!=stopc)
			error("Nonterminated font name");
			continue;
		case 'P': case 'p':
			if (icol<=0)
				continue;
			temp = snp = csize[nclin][icol-1];
			while ((c = get1char())) {
				if (c== ' ' || c== tab || c=='\n')
					break;
				if (c=='-' || c == '+') {
					if (snp>temp)
						break;
					else
						*snp++=c;
				} else if (isdigit(c))
					*snp++ = c;
				else
					break;
				if (snp-temp>4)
					error("point size too large");
			}
			*snp = 0;
			if (atoi(temp)>36)
				error("point size unreasonable");
			un1getc (c);
			continue;
		case 'V': case 'v':
			if (icol<=0)
				continue;
			temp = snp = vsize[nclin][icol-1];
			while ((c = get1char())) {
				if (c== ' ' || c== tab || c=='\n') break;
				if (c=='-' || c == '+')
				if (snp>temp)
				break;
				else
				*snp++=c;
				else
				if (isdigit(c))
					*snp++ = c;
				else
					break;
				if (snp-temp>4)
					error("vertical spacing value too large");
			}
			*snp=0;
			un1getc(c);
			continue;
		case 'w': case 'W':
			snp = cll [icol-1];
#if 0
			/* Dale Smith didn't like this check -
			 * possible to have two text blocks
			 * of different widths now ....
			 */
			if (*snp) {
				fprintf(stderr,
				    "Ignored second width specification");
				continue;
			}
#endif
			stopc=0;
			while ((c = get1char())) {
				if (snp == cll[icol-1] && c == '(') {
					stopc = ')';
					continue;
				}
				if ( !stopc && (c >'9' || c < '0'))
					break;
				if (stopc && c == stopc)
					break;
				*snp++ = c;
			}
			*snp=0;
			if (snp - cll[icol - 1] > CLLEN)
				error ("column width too long");
			if (!stopc)
				un1getc(c);
			continue;
		case 'e': case 'E':
			if (icol<1)
				continue;
			evenup[icol - 1]=1;
			evenflg=1;
			continue;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			sn[0] = c;
			snp=sn+1;
			while (isdigit(*snp++ = c = get1char()))
				;
			un1getc(c);
			sep[icol-1] = MAX(sep[icol - 1], atoi(sn));
			continue;
		case '|':
			lefline[nclin][icol]++;
			if (icol == 0)
				left1flg=1;
			continue;
		}
	}
	error("EOF reading table specification");
}

/* t5.c: read data for table */

void
gettbl(void)
{
	int icol, ch;

	cstore = cspace = chspace();
	textflg = 0;
	for (nlin = nslin = 0; gets1(cstore); nlin++) {
		stynum[nlin] = nslin;
		if (prefix(".TE", cstore)) {
			leftover = NULL;
			break;
		}
		if (prefix(".TC", cstore) || prefix(".T&", cstore)) {
			readspec();
			nslin++;
		}
		if (nlin >= MAXLIN) {
			leftover = cstore;
			break;
		}
		fullbot[nlin]=0;
		if (cstore[0] == '.' && !isdigit(cstore[1])) {
			instead[nlin] = cstore;
			while (*cstore++);
			continue;
		} else
			instead[nlin] = 0;
		if (nodata(nlin)) {
			if ((ch = oneh(nlin)))
				fullbot[nlin]= ch;
			nlin++;
			nslin++;
			instead[nlin] = NULL;
			fullbot[nlin] = 0;
		}
		table[nlin] = (struct colstr *)alocv((ncol+2)*sizeof(table[0][0]));
		if (cstore[1]==0)
			switch(cstore[0]) {
			case '_': fullbot[nlin]= '-'; continue;
			case '=': fullbot[nlin]= '='; continue;
			}
		stynum[nlin] = nslin;
		nslin = MIN(nslin+1, nclin-1);
		for (icol = 0; icol <ncol; icol++) {
			table[nlin][icol].col = cstore;
			table[nlin][icol].rcol=0;
			ch=1;
			if (match(cstore, "T{")) /* text follows */
				table[nlin][icol].col =
				    (char *)gettext(cstore, nlin, icol,
				    font[stynum[nlin]][icol],
				    csize[stynum[nlin]][icol]);
			else {
				for(; (ch= *cstore) != '\0' && ch != tab;
				    cstore++)
					;
				*cstore++ = '\0';
				/* numerical or alpha, subcol */
				switch(ctype(nlin,icol)) {
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
			if (ch == '\0')
				break;
		}
		while (++icol <ncol+2) {
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
	if (textflg)
		untext();
	return;
}

int
nodata(int il)
{
	int c;
	for (c=0; c<ncol;c++) {
		switch(ctype(il,c)) {
		case 'c': case 'n': case 'r': case 'l': case 's': case 'a':
			return(0);
		}
	}
	return(1);
}

int
oneh(int lin)
{
	int k, icol;
	k = ctype(lin,0);
	for(icol=1; icol<ncol; icol++) {
		if (k != ctype(lin,icol))
			return(0);
	}
	return(k);
}

# define SPAN "\\^"
void
permute(void)
{
	int irow, jcol, is;
	char *start, *strig;
	for(jcol=0; jcol<ncol; jcol++) {
		for(irow=1; irow<nlin; irow++) {
			if (vspand(irow,jcol,0)) {
				is = prev(irow);
				if (is < 0)
					error("Vertical spanning in first row not allowed");
				start = table[is][jcol].col;
				strig = table[is][jcol].rcol;
				while (irow<nlin && vspand(irow,jcol,0))
					irow++;
				table[--irow][jcol].col = start;
				table[irow][jcol].rcol = strig;
				while (is < irow) {
					table[is][jcol].rcol =0;
					table[is][jcol].col= SPAN;
					is = next(is);
				}
			}
		}
	}
}

int
vspand(int ir, int ij, int ifform)
{
	if (ir < 0)
		return(0);
	if (ir >= nlin)
		return(0);
	if (instead[ir])
		return(0);
	if (ifform == 0 && ctype(ir,ij)=='^')
		return(1);
	if (!table[ir] || table[ir][ij].rcol != 0)
		return (0);
	if (fullbot[ir])
		return(0);
	return(vspen(table[ir][ij].col));
}

int
vspen(char *s)
{
	if (s==0)
		return(0);
	if (!point(s))
		return(0);
	return(match(s, SPAN));
}

/* t6.c: compute tab stops */

void
maktab(void)
{
# define FN(i,c) font[stynum[i]][c]
# define SZ(i,c) csize[stynum[i]][c]
	/* define the tab stops of the table */
	int icol, ilin, tsep, k, ik, vforml, il, text;
	int doubled[MAXCOL], acase[MAXCOL];
	char *s;
	for(icol=0; icol <ncol; icol++) {
		doubled[icol] = acase[icol] = 0;
		printf(".nr %d 0\n", icol + CRIGHT);
		for(text = 0; text < 2; text++) {
			if (text)
				printf(".%02d\n.rm %02d\n",
				    icol + 80, icol + 80);
			for(ilin = 0; ilin < nlin; ilin++) {
				if (instead[ilin]|| fullbot[ilin])
					continue;
				vforml=ilin;
				for(il=prev(ilin);
				    il>=0 && vspen(table[il][icol].col);
				    il=prev(il))
					vforml=il;
				if (fspan(vforml,icol))
					continue;
				if (filler(table[ilin][icol].col))
					continue;
				switch(ctype(vforml,icol)) {
				case 'a':
					acase[icol]=1;
					s = table[ilin][icol].col;
					if (tx(s) && text) {
						if (doubled[icol]==0)
							printf(".nr %d 0\n.nr %d 0\n",S1,S2);
						doubled[icol]=1;
						printf(".if \\n(%c->\\n(%d .nr %d \\n(%c-\n",s,S2,S2,s);
					}
				case 'n':
					if (table[ilin][icol].rcol!=0) {
						if (doubled[icol]==0 && text==0)
						printf(".nr %d 0\n.nr %d 0\n", S1, S2);
						doubled[icol]=1;
						if (real(s=table[ilin][icol].col) && !vspen(s)) {
							if (tx(s) != text) continue;
							printf(".nr %d ", TMP);
							wide(s, FN(vforml,icol), SZ(vforml,icol)); putchar('\n');
							printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n", S1, TMP, S1, TMP);
						}
						if (text==0 && real(s=table[ilin][icol].rcol) && !vspen(s) && !barent(s)) {
							printf(".nr %d \\w%c%s%c\n",TMP, F1, s, F1);
							printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n",S2,TMP,S2,TMP);
						}
						continue;
					}
				case 'r':
				case 'c':
				case 'l':
					if (real(s=table[ilin][icol].col) && !vspen(s)) {
						if (tx(s) != text)
							continue;
						printf(".nr %d ", TMP);
						wide(s, FN(vforml,icol), SZ(vforml,icol)); putchar('\n');
						printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n", icol+CRIGHT, TMP, icol+CRIGHT, TMP);
					}
				}
			}
		}
		if (acase[icol])
			printf(".if \\n(%d>=\\n(%d .nr %d \\n(%du+2n\n",S2,icol+CRIGHT,icol+CRIGHT,S2);
		if (doubled[icol]) {
			printf(".nr %d \\n(%d\n", icol+CMID, S1);
			printf(".nr %d \\n(%d+\\n(%d\n",TMP,icol+CMID,S2);
			printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n",TMP,icol+CRIGHT,icol+CRIGHT,TMP);
			printf(".if \\n(%d<\\n(%d .nr %d +(\\n(%d-\\n(%d)/2\n",TMP,icol+CRIGHT,icol+CMID,icol+CRIGHT,TMP);
		}
		if (cll[icol][0]) {
			printf(".nr %d %sn\n", TMP, cll[icol]);
			printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n",icol+CRIGHT, TMP, icol+CRIGHT, TMP);
		}
		for(ilin=0; ilin < nlin; ilin++)
			if ((k = lspan(ilin, icol))) {
				s=table[ilin][icol-k].col;
				if (!real(s) || barent(s) || vspen(s) )
					continue;
				printf(".nr %d ", TMP);
				wide(table[ilin][icol-k].col, FN(ilin,icol-k), SZ(ilin,icol-k));
				for(ik=k; ik>=0; ik--) {
					printf("-\\n(%d",CRIGHT+icol-ik);
					if (!expflg && ik>0)
						printf("-%dn", sep[icol-ik]);
				}
				printf("\n");
				printf(".if \\n(%d>0 .nr %d \\n(%d/%d\n", TMP, TMP, TMP, k);
				printf(".if \\n(%d<0 .nr %d 0\n", TMP, TMP);
				for(ik = 1; ik <= k; ik++) {
					if (doubled[icol-k+ik])
						printf(".nr %d +\\n(%d/2\n", icol-k+ik+CMID, TMP);
					printf(".nr %d +\\n(%d\n", icol-k+ik+CRIGHT, TMP);
				}
			}
	}
	if (textflg)
		untext();
	/* if even requested, make all columns widest width */
# define TMP1 S1
# define TMP2 S2
	if (evenflg) {
		printf(".nr %d 0\n", TMP);
		for(icol=0; icol<ncol; icol++) {
			if (evenup[icol]==0)
				continue;
			printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n",
					icol+CRIGHT, TMP, TMP, icol+CRIGHT);
		}
		for(icol=0; icol<ncol; icol++) {
			if (evenup[icol]==0)
			/* if column not evened just retain old interval */
				continue;
			if (doubled[icol])
				printf(".nr %d (100*\\n(%d/\\n(%d)*\\n(%d/100\n",
				    icol+CMID, icol+CMID, icol+CRIGHT, TMP);
			/* that nonsense with the 100's and parens tries
			 to avoid overflow while proportionally shifting
			 the middle of the number */
			printf(".nr %d \\n(%d\n", icol+CRIGHT, TMP);
		}
	}
	/* now adjust for total table width */
	for(tsep=icol=0; icol<ncol; icol++)
		tsep+= sep[icol];
	if (expflg) {
		printf(".nr %d 0", TMP);
		for(icol=0; icol<ncol; icol++)
			printf("+\\n(%d", icol+CRIGHT);
		printf("\n");
		printf(".nr %d \\n(.l-\\n(%d\n", TMP, TMP);
		if (boxflg || dboxflg || allflg)
			tsep += 1;
		else
			tsep -= sep[ncol-1];
		printf(".nr %d \\n(%d/%d\n", TMP, TMP, tsep);
		printf(".if \\n(%d<0 .nr %d 0\n", TMP, TMP);
	} else
		printf(".nr %d 1n\n", TMP);
	printf(".nr %d 0\n",CRIGHT-1);
	tsep= (boxflg || allflg || dboxflg || left1flg) ? 1 : 0;
	for (icol = 0; icol < ncol; icol++) {
		printf(".nr %d \\n(%d+(%d*\\n(%d)\n",icol+CLEFT, icol+CRIGHT-1, tsep, TMP);
		printf(".nr %d +\\n(%d\n",icol+CRIGHT, icol+CLEFT);
		if (doubled[icol]) {
			/* the next line is last-ditch effort to avoid zero field width */
			/*printf(".if \\n(%d=0 .nr %d 1\n",icol+CMID, icol+CMID);*/
			printf(".nr %d +\\n(%d\n", icol+CMID, icol+CLEFT);
			/*  printf(".if n .if \\n(%d%%24>0 .nr %d +12u\n",icol+CMID, icol+CMID); */
		}
		tsep=sep[icol];
	}
	if (rightl)
		printf(".nr %d (\\n(%d+\\n(%d)/2\n",ncol+CRIGHT-1, ncol+CLEFT-1, ncol+CRIGHT-2);
	printf(".nr TW \\n(%d\n", ncol+CRIGHT-1);
	if (boxflg || allflg || dboxflg)
		printf(".nr TW +%d*\\n(%d\n", sep[ncol-1], TMP);
	printf(".if t .if \\n(TW>\\n(.li .tm Table at line %d file %s is too wide - \\n(TW units\n", iline-1, ifile);
}

void
wide(const char *s, const char *fn, const char *size)
{
	if (point(s)) {
		printf("\\w%c", F1);
		if (*fn>0)
			putfont(fn);
		if (*size)
			putsize(size);
		printf("%s", s);
		if (*fn>0)
			putfont("P");
		if (*size)
			putsize("0");
		printf("%c", F1);
	} else
		printf("\\n(%c-", s);
}

int
filler(const char *s)
{
	return (point(s) && s[0] == '\\' && s[1] == 'R');
}

/* t7.c: control to write table entries */

#define realsplit ((ct=='a'||ct=='n') && table[ldata][c].rcol)

void
runout(void)
{
	int i;

	if (boxflg || allflg || dboxflg)
		need();
	if (ctrflg) {
		printf(".nr #I \\n(.i\n");
		printf(".in +(\\n(.lu-\\n(TWu-\\n(.iu)/2u\n");
	}
	printf(".fc %c %c\n", F1, F2);
	printf(".nr #T 0-1\n");
	deftail();
	for(i=0; i<nlin; i++)
		putline(i,i);
	if (leftover)
		yetmore();
	printf(".fc\n");
	printf(".nr T. 1\n");
	printf(".T# 1\n");
	if (ctrflg)
		printf(".in \\n(#Iu\n");
}

void
runtabs(int lform, int ldata)
{
	int c, ct, vforml, lf;

	printf(".ta ");
	for(c = 0; c < ncol; c++) {
		vforml = lform;
		for (lf = prev(lform); lf >= 0 && vspen(table[lf][c].col);
		    lf = prev(lf))
			vforml = lf;
		if (fspan(vforml, c))
			continue;
		switch(ct = ctype(vforml, c)) {
		case 'n':
		case 'a':
			if (table[ldata][c].rcol)
				if (lused[c]) /*Zero field width*/
					printf("\\n(%du ", c + CMID);
		case 'c':
		case 'l':
		case 'r':
			if (realsplit? rused[c]: (used[c]+lused[c]))
				printf("\\n(%du ", c + CRIGHT);
			continue;
		case 's':
			if (lspan(lform, c))
				printf("\\n(%du ", c + CRIGHT);
			continue;
		}
	}
	putchar('\n');
}

int
ifline(const char *s)
{
	if (!point(s))
		return(0);
	if (s[0] == '\\')
		s++;
	if (s[1])
		return(0);
	if (s[0] == '_')
		return('-');
	if (s[0] == '=')
		return('=');
	return(0);
}

void
need(void)
{
	int texlin, horlin, i;

	for(texlin = horlin = i = 0; i < nlin; i++) {
		if (fullbot[i] != 0)
			horlin++;
		else if (instead[i] != 0)
			continue;
		else if (allh(i))
			horlin++;
		else
			texlin++;
	}
	printf(".ne %dv+%dp\n", texlin, 2 * horlin);
}

void
deftail(void)
{
	int i, c, lf, lwid;
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			printf(".nr #%c 0-1\n", linestop[i] + 'a' - 1);
	puts(".nr #a 0-1\n"
	    ".eo\n"
	    ".de T#\n"
	    ".ds #d .d\n"
	    ".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n"
	    ".mk ##\n"
	    ".nr ## -1v\n"
	    ".ls 1");
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			printf(".if \\n(#T>=0 .nr #%c \\n(#T\n",
			    linestop[i] + 'a' - 1);
	if (boxflg || allflg || dboxflg) /* bottom of table line */
		if (fullbot[nlin-1]==0) {
			if (!pr1403)
				puts(".if \\n(T. .vs \\n(.vu-\\n(.sp");
			printf(".if \\n(T. ");
			drawline(nlin,0,ncol, dboxflg ? '=' : '-',1,0);
			puts("\n.if \\n(T. .vs");
			/* T. is really an argument to a macro but because of 
			 eqn we don't dare pass it as an argument and reference by $1 */
		}
	for(c=0; c<ncol; c++) {
		if ((lf=left(nlin-1,c, &lwid))>=0) {
			printf(".if \\n(#%c>=0 .sp -1\n",
			    linestop[lf] + 'a' - 1);
			printf(".if \\n(#%c>=0 ", linestop[lf] + 'a' - 1);
			tohcol(c);
			drawvert(lf, nlin-1, c, lwid);
			puts("\\h'|\\n(TWu'");
		}
	}
	if (boxflg || allflg || dboxflg) /* right hand line */ {
		printf(".if \\n(#a>=0 .sp -1\n"
		    ".if \\n(#a>=0 \\h'|\\n(TWu'");
		drawvert (0, nlin-1, ncol, dboxflg? 2 : 1);
		putchar('\n');
	}
	puts(".ls\n..\n.ec");
}

/* tb.c: check which entries exist, also storage allocation */

void
checkuse(void)
{
	int i, c, k;

	for(c = 0; c<ncol; c++) {
		used[c] = lused[c] = rused[c] = 0;
		for(i = 0; i < nlin; i++) {
			if (instead[i] || fullbot[i])
				continue;
			k = ctype(i,c);
			if (k == '-' || k == '=')
				continue;
			if ((k == 'n'||k == 'a')) {
				rused[c]|= real(table[i][c].rcol);
				if (!real(table[i][c].rcol))
				used[c] |= real(table[i][c].col);
				if (table[i][c].rcol)
				lused[c] |= real(table[i][c].col);
			} else
				used[c] |= real(table[i][c].col);
		}
	}
}

int
real(const char *s)
{
	if (s == 0)
		return(0);
	if (!point(s))
		return(1);
	if (*s == 0)
		return(0);
	return(1);
}

/* tc.c: find character not in table to delimit fields */

void
choochar(void)
{
	/* choose funny characters to delimit fields */
	int had[128], ilin,icol, k;
	char *s;

	for(icol=0; icol<128; icol++)
		had[icol] = 0;
	F1 = F2 = 0;
	for(ilin=0;ilin<nlin;ilin++) {
		if (instead[ilin])
			continue;
		if (fullbot[ilin])
			continue;
		for(icol = 0; icol<ncol; icol++) {
			k = ctype(ilin, icol);
			if (k==0 || k == '-' || k == '=')
				continue;
			s = table[ilin][icol].col;
			if (point(s))
				while (*s)
					had[*s++]=1;
			s = table[ilin][icol].rcol;
			if (point(s))
				while (*s)
					had[*s++]=1;
		}
	}
	/* choose first funny character */
	for(s="\002\003\005\006\007!%&#/?,:;<=>@`^~_{}+-*ABCDEFGHIJKMNOPQRSTUVWXYZabcdefgjkoqrstwxyz";
	    *s; s++) {
		if (had[*s]==0) {
			F1= *s;
			had[F1]=1;
			break;
		}
	}
	/* choose second funny character */
	for(s="\002\003\005\006\007:_~^`@;,<=>#%&!/?{}+-*ABCDEFGHIJKMNOPQRSTUVWXZabcdefgjkoqrstuwxyz";
	    *s; s++) {
		if (had[*s] == 0) {
			F2 = *s;
			break;
		}
	}
	if (F1 == 0 || F2 == 0)
		error("couldn't find characters to use for delimiters");
}

/* tf.c: save and restore fill mode around table */

void
rstofill(void)
{
	printf(".%d\n", SF);
}

/* tu.c: draws horizontal lines */

void
makeline(int i, int c, int lintype)
{
	int cr, type, shortl;

	type = thish(i,c);
	if (type == 0) 
		return;
	cr = c;
	shortl = (table[i][c].col[0]=='\\');
	if (c > 0 && !shortl && thish(i, c - 1) == type)
		return;
	if (shortl == 0)
		for (cr = c; cr < ncol && (ctype(i,cr) == 's' ||
		    type == thish(i,cr)); cr++)
			;
	else
		for (cr = c + 1; cr < ncol && ctype(i, cr) == 's'; cr++)
			;
	drawline(i, c, cr - 1, lintype, 0, shortl);
}

void
fullwide(int i, int lintype)
{
	int cr, cl;

	if (!pr1403)
		printf(".nr %d \\n(.v\n.vs \\n(.vu-\\n(.sp\n", SVS);
	cr = 0;
	while (cr < ncol) {
		cl = cr;
		while (i > 0 && vspand(prev(i), cl, 1))
			cl++;
		for(cr = cl; cr < ncol; cr++)
			if (i > 0 && vspand(prev(i), cr, 1))
				break;
		if (cl<ncol)
			drawline(i, cl,(cr < ncol? cr - 1 : cr), lintype, 1, 0);
	}
	putchar('\n');
	if (!pr1403)
		printf(".vs \\n(%du\n", SVS);
}

void
drawline(int i, int cl, int cr, int lintype, int noheight, int shortl)
{
	char *exhr, *exhl, *lnch;
	int lcount, ln, linpos, oldpos, nodata;

	lcount=0;
	exhr=exhl= "";
	switch(lintype) {
	case '-': lcount=1;break;
	case '=': lcount = pr1403? 1 : 2; break;
	case SHORTLINE: lcount=1; break;
	}

	if (lcount<=0)
		return;
	nodata = cr-cl>=ncol || noheight || allh(i);
	if (!nodata)
		printf("\\v'-.5m'");
	for(ln=oldpos=0; ln<lcount; ln++) {
		linpos = 2*ln - lcount +1;
		if (linpos != oldpos)
			printf("\\v'%dp'", linpos - oldpos);
		oldpos=linpos;
		if (shortl==0) {
			tohcol(cl);
			if (lcount>1) {
				switch(interv(i,cl)) {
				case TOP: exhl = ln==0 ? "1p" : "-1p"; break;
				case BOT: exhl = ln==1 ? "1p" : "-1p"; break;
				case THRU: exhl = "1p"; break;
				}
				if (exhl[0])
					printf("\\h'%s'", exhl);
			} else if (lcount==1) {
				switch(interv(i, cl)) {
				case TOP: case BOT: exhl = "-1p"; break;
				case THRU: exhl = "1p"; break;
				}
				if (exhl[0])
					printf("\\h'%s'", exhl);
			}

			if (lcount > 1) {
				switch(interv(i,cr+1)) {
				case TOP: exhr = ln==0 ? "-1p" : "+1p"; break;
				case BOT: exhr = ln==1 ? "-1p" : "+1p"; break;
				case THRU: exhr = "-1p"; break;
				}
			}
			else if (lcount==1) {
				switch(interv(i,cr+1)) {
				case TOP: case BOT: exhr = "+1p"; break;
				case THRU: exhr = "-1p"; break;
				}
			}
		} else
			printf("\\h'|\\n(%du'", cl + CLEFT);
		printf("\\s\\n(%d", LSIZE);
		if (linsize)
			printf("\\v'-\\n(%dp/6u'", LSIZE);
		if (shortl)
			printf("\\l'|\\n(%du'", cr + CRIGHT);
		else {
			lnch = "\\(ul";
			if (pr1403)
				lnch = lintype==2 ? "=" : "\\(ru";
			if (cr + 1 >= ncol)
				printf("\\l'|\\n(TWu%s%s'", exhr, lnch);
			else
				printf("\\l'(|\\n(%du+|\\n(%du)/2u%s%s'",
				    cr + CRIGHT, cr + 1 + CLEFT, exhr, lnch);
		}
		if (linsize)
			printf("\\v'\\n(%dp/6u'", LSIZE);
		printf("\\s0");
	}
	if (oldpos!=0)
		printf("\\v'%dp'", -oldpos);
	if (!nodata)
		printf("\\v'+.5m'");
}

void
getstop(void)
{
	int i, c, k, junk, stopp;

	stopp = 1;
	for(i = 0; i < MAXLIN; i++)
		linestop[i] = 0;
	for(i = 0; i < nlin; i++)
		for(c = 0; c < ncol; c++) {
			k = left(i, c, &junk);
			if (k >= 0 && linestop[k] == 0)
				linestop[k] = ++stopp;
		}
	if (boxflg || allflg || dboxflg)
		linestop[0] = 1;
}
