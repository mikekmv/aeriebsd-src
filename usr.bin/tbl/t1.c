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
static char sccsid[] = "@(#)t1.c	4.5 (Berkeley) 4/18/91";
#endif /* not lint */

/* t1.c: main control and input switching */

#include "tbl.h"
#include <signal.h>
#include "pathnames.h"

# ifdef gcos
/* required by GCOS because file is passed to "tbl" by troff preprocessor */
# define _f1 _f
extern FILE *_f[];
# endif

# define ever (;;)

main(argc,argv)
char *argv[];
{
# ifdef unix
	void badsig();
	signal(SIGPIPE, badsig);
# endif
# ifdef gcos
	if(!intss()) tabout = fopen("qq", "w"); /* default media code is type 5 */
# endif
	exit(tbl(argc,argv));
}

tbl(argc,argv)
char *argv[];
{
	char line[BUFSIZ];
	/* required by GCOS because "stdout" is set by troff preprocessor */
	tabin=stdin; tabout=stdout;
	setinp(argc,argv);
	while (gets1(line))
	{
		fprintf(tabout, "%s\n",line);
		if (prefix(".TS", line))
		tableput();
	}
	fclose(tabin);
	return(0);
}
int sargc;
char **sargv;
setinp(argc,argv)
char **argv;
{
	sargc = argc;
	sargv = argv;
	sargc--; sargv++;
	if (sargc>0)
	swapin();
}
swapin()
{
	while (sargc>0 && **sargv=='-') /* Mem fault if no test on sargc */
	{
		if (sargc<=0) return(0);
		if (match("-ms", *sargv))
		{
			*sargv = _PATH_MACROS;
			break;
		}
		if (match("-mm", *sargv))
		{
			*sargv = _PATH_PYMACS;
			break;
		}
		if (match("-TX", *sargv))
		pr1403=1;
		sargc--; sargv++;
	}
	if (sargc<=0) return(0);
# ifdef unix
	/* file closing is done by GCOS troff preprocessor */
	if (tabin!=stdin) fclose(tabin);
# endif
	tabin = fopen(ifile= *sargv, "r");
	iline=1;
# ifdef unix
	/* file names are all put into f. by the GCOS troff preprocessor */
	fprintf(tabout, ".ds f. %s\n",ifile);
# endif
	if (tabin==NULL)
	error("Can't open file");
	sargc--;
	sargv++;
	return(1);
}
# ifdef unix
void
badsig()
{
	signal(SIGPIPE, SIG_IGN);
	exit(0);
}
# endif
