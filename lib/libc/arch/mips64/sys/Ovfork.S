/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
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
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: Ovfork.S,v 1.3 2009/10/31 21:30:59 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include "SYS.h"

/*
 * pid = vfork();
 *
 * v1 == 0 in parent process, v1 == 1 in child process.
 * v0 == pid of child in parent, v0 == pid of parent in child.
 */

FRAMESZ= MKFSIZ(1,1)
GPOFF= FRAMESZ-2*REGSZ

SYSLEAF(vfork, FRAMESZ)
	PTR_SUBU sp, FRAMESZ
	SETUP_GP64(GPOFF, vfork)
	.set	reorder
	li	v0, SYS_vfork		# system call number for vfork
	syscall
	beq	a3, zero, 1f		# jump if no errors
	LA	t9, cerror
	RESTORE_GP64
	PTR_ADDU sp, FRAMESZ
	jr	t9
1:
	beq	v1, zero, 2f		# parent process ?
	move	v0, zero		# return zero in child
2:
	RESTORE_GP64
	PTR_ADDU sp, FRAMESZ
	j	ra
SYSEND(vfork)
