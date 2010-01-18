/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	from: @(#)exec.h	8.1 (Berkeley) 6/11/93
 *	from: imgact_aout.h,v 1.2 1994/12/30 08:06:19 bde Exp
 */

#ifndef	_OPENBSD_EXEC_H
#define	_OPENBSD_EXEC_H

#define	OPENBSD_AOUT_HDR_SIZE	sizeof(struct exec)
#define OPENBSD_N_GETMAGIC(ex)	((ex).a_midmag & 0xffff)
#define OPENBSD_N_GETMID(ex)	(((ex).a_midmag >> 16) & 0x03ff)

#define OPENBSD_ELF_AUX_ARGSIZ	(sizeof(AuxInfo) * 8)

int openbsd_elf32_probe(struct proc *, struct exec_package *, char *,
    u_long *, u_int8_t *);
int openbsd_elf64_probe(struct proc *, struct exec_package *, char *,
    u_long *, u_int8_t *);
int openbsd_elf32_makecmds(struct proc *, struct exec_package *);
int exec_openbsd_aout_makecmds(struct proc *, struct exec_package *);

extern char openbsd_sigcode[], openbsd_esigcode[];

extern struct emul emul_openbsd_aout, emul_openbsd_elf32, emul_openbsd_elf64;

#endif /* !_OPENBSD_EXEC_H */
