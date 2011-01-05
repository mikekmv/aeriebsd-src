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
static char sccsid[] = "@(#)n2.c	4.2 (Berkeley) 4/18/91";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif /* not lint */

#include <sgtty.h>
#include <fcntl.h>

#include "tdef.h"
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>

/*
n2.c

output, cleanup
*/

int toolate;
int error;
jmp_buf sjbuf;

int
pchar(int c)
{
	int i, j;

	if ((i = c) & MOT) {
		pchar1(i);
		return (0);
	}
	switch ((j = i & CMASK)) {
	case 0:
	case IMP:
	case RIGHT:
	case LEFT:
		return (0);
	case HX:
		j = (tlss>>9) | ((i&~0777)>>3);
		if(i & 040000) {
			j &= ~(040000>>3);
			if(j > dip->blss)
				dip->blss = j;
		} else {
			if(j > dip->alss)
				dip->alss = j;
			ralss = dip->alss;
		}
		tlss = 0;
		return (0);
	case LX:
		tlss = i;
		return (0);
	case PRESC:
		if (dip == &d[0])
			j = eschar;
	default:
		i = (trtab[j] & BMASK) | (i & ~CMASK);
	}
	pchar1 (i);
	return (0);
}

void
pchar1(int c)
{
#ifndef NROFF
	extern const int chtab[];
	const int *k;
#endif
	int i, j;

	j = (i = c) & CMASK;
	if(dip != &d[0]){
		wbf(i);
		dip->op = offset;
		return;
	}
	if (!tflg && !print) {
		if (j == '\n')
			dip->alss = dip->blss = 0;
		return;
	}
	if (no_out || (j == FILLER))
		return;
#ifndef NROFF
	if(ascii){
		if(i & MOT){
			oput(' ');
			return;
		}
		if(j < 0177){
			oput(i);
			return;
		}
		switch (j) {
		case 0200:
		case 0210:
			oput('-');
			break;
		case 0211:
			oputs("fi");
			break;
		case 0212:
			oputs("fl");
			break;
		case 0213:
			oputs("ff");
			break;
		case 0214:
			oputs("ffi");
			break;
		case 0215:
			oputs("ffl");
			break;
		default:
			for (k = chtab; *++k != j; k++)
				if (*k == 0)
					return;
			oput('\\');
			oput('(');
			oput(*--k & BMASK);
			oput(*k >> BYTE);
		}
	} else
#endif
	ptout(i);
}

void
oput(int i)
{
	*obufp++ = i;
	if (obufp == (obuf + OBUFSZ + ascii - 1))
		flusho();
}

void
oputs(char *i)
{
	while (*i != 0)
		oput(*i++);
}

void
flusho(void)
{
	if (!ascii)
		*obufp++ = 0;

	if (!ptid){
		while((ptid = open(ptname, O_WRONLY)) < 0){
			if(++waitf <=2)
				prstr("Waiting for Typesetter.\n");
			sleep(15);
		}
	}
	if (no_out == 0){
		if (!toolate) {
			toolate++;
#ifdef NROFF
			if(t.bset || t.breset){
				if(ttysave == -1) {
					gtty(1, &ttys);
					ttysave = ttys.sg_flags;
				}
				ttys.sg_flags &= ~t.breset;
				ttys.sg_flags |= t.bset;
				stty(1, &ttys);
			}
			{
			char *p = t.twinit;
			while (*p++)
				;
			write(ptid, t.twinit, p - t.twinit - 1);
			}
#endif
		}
		toolate += write(ptid, obuf, obufp - obuf);
	}
	obufp = obuf;
}

void
done(int x)
{
	int i;

	error |= x;
	level = 0;
	app = ds = lgf = 0;
	if ((i = em)) {
		donef = -1;
		em = 0;
		if (control(i,0))
			longjmp(sjbuf,1);
	}
	if(!nfo)
		done3(0);
	mflg = 0;
	dip = &d[0];
	if(woff)
		wbt(0);
	if(pendw)
		getword(1);
	pendnf = 0;
	if(donef == 1)
		done1(0);
	donef = 1;
	ip = 0;
	frame = stk;
	nxf = frame + 1;
	if(!ejf)
		tbreak();
	nflush++;
	eject((struct s *)0);
	longjmp(sjbuf,1);
}

void
done1(int x)
{
	error |= x;
	if(v.nl){
		trap = 0;
		eject((struct s *)0);
		longjmp(sjbuf,1);
	}
	if(nofeed){
		ptlead();
		flusho();
		done3(0);
	}else{
		if(!gflag)
			lead += TRAILER;
		done2(0);
	}
}

void
done2(int x)
{
	ptlead();
#ifndef NROFF
	if (!ascii) {
		int i;

		oput(T_INIT);
		oput(T_STOP);
		if (!gflag)
			for (i = 8; i > 0; i--)
				oput(T_PAD);
		if (stopmesg)
			prstr("Troff finished.\n");
	}
#endif
	flusho();
	done3(x);
}

void
done3(int x)
{
	error |= x;
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	if (unlkp)
		unlink(unlkp);
#ifdef NROFF
	twdone();
#endif
	if(quiet){
		ttys.sg_flags |= ECHO;
		stty(0, &ttys);
	}
#ifndef NROFF
	report();
#endif
	exit(error);
}

void
edone(int x)
{
	frame = stk;
	nxf = frame + 1;
	ip = 0;
	done(x);
}

#ifndef NROFF
void
report(void)
{
	struct {int use; int uid;} a;

	if ((ptid != 1) && paper ) {
		lseek(acctf,0L,2);
		a.use = paper;
		a.uid = getuid();
		write(acctf,(char *)&a,sizeof(a));
		close(acctf);
	}
}
#endif

#ifdef NROFF
void
casepi(void)
{
	int i, id[2];

	if (toolate || skip() || !getname() || pipe(id) == -1 ||
	    (i = fork()) == -1) {
		prstr("Pipe not created.\n");
		return;
	}
	ptid = id[1];
	if (i > 0) {
		close(id[0]);
		toolate++;
		pipeflg++;
		return;
	}
	close(0);
	dup(id[0]);
	close(id[1]);
	execl(nextf, nextf, NULL);
	prstr("Cannot exec: ");
	prstr(nextf);
	prstr("\n");
	exit(-4);
}
#endif
