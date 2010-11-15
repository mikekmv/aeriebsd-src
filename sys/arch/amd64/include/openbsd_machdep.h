/*
 * Copyright (c) 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)signal.h	8.1 (Berkeley) 6/11/93
 *	from: Id: signal.h,v 1.4 1994/08/21 04:55:30 paul Exp 
 *
 *	from: @(#)frame.h	5.2 (Berkeley) 1/18/91
 *	from: Id: frame.h,v 1.10 1995/03/16 18:11:42 bde Exp 
 */
#ifndef _OPENBSD_MACHDEP_H
#define _OPENBSD_MACHDEP_H

/* sys/i386/include/exec.h */
#define OPENBSD___LDPGSZ	4096

/*
 * signal support
 */

struct	openbsd_sigcontext {
	/* plain match trapframe */
	long	sc_rdi;
	long	sc_rsi;
	long	sc_rdx;
	long	sc_rcx;
	long	sc_r8;
	long	sc_r9;
	long	sc_r10;
	long	sc_r11;
	long	sc_r12;
	long	sc_r13;
	long	sc_r14;
	long	sc_r15;
	long	sc_rbp;
	long	sc_rbx;
	long	sc_rax;
	long	sc_gs;
	long	sc_fs;
	long	sc_es;
	long	sc_ds;
	long	sc_trapno;
	long	sc_err;
	long	sc_rip;
	long	sc_cs;
	long	sc_rflags;
	long	sc_rsp;
	long	sc_ss;

	struct fxsave64 *sc_fpstate;
	int	sc_onstack;
	int	sc_mask;
};

#ifdef _KERNEL
void openbsd_sendsig(sig_t, int, int, u_long, int, union sigval);
#endif

#endif /* _OPENBSD_MACHDEP_H */
