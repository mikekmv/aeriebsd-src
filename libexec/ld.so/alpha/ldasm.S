
/*
 * Copyright (c) 2001 Niklas Hallqvist
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*
 * Copyright 1996 Matt Thomas <matt@3am-software.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <machine/asm.h>
#include <machine/pal.h>
#include <sys/syscall.h>

#define AUX_entry 9

	.extern _GLOBAL_OFFSET_TABLE_

/* Not really a leaf... but we are special.  */
LEAF_NOPROFILE(_dl_start, 0)
	.set	noreorder
	br	pv, L1
L1:
	LDGP(pv)

	mov	a0, s0		/* save arg */

	/* relocate ourself. */
	br	s2, L2		/* get our PC */
L2:	ldiq	s3, L2		/* get where the linker thought we were */

	subq	s2, s3, s2
	mov	s2, a1
	lda	t5, _DYNAMIC
	addq	s2, t5, a0

	bsr	ra, _reloc_alpha_got

	/* allocate stack */
	lda	sp, (-8 - ((AUX_entry) * 8))(sp)

	mov	s0, a0
	mov	s2, s1		/* relocation displacement */
	ldq	a2, 0(a0)	/* argc */
	lda	a3, 8(a0)	/* argv */
	mov	a3, s3
	lda	t3, 1(a2)
	sll	t3, 3, t3
	addq	a3, t3, a4	/* envp */
	mov	a4, s4
	mov	a5, s5
	lda	s2, 0(sp)
	mov	s2, a1
	mov	0,  a2		/* dynamicp is unused on alpha */
	CALL(_dl_boot_bind)
	mov	s3, a0		/* **argv  */
	mov	s4, a1		/* **envp  */
	mov	s1, a2		/* loff    */
	mov	s2, a3		/* dl_data */
	CALL(_dl_boot)
	mov	s0, a0
	mov	v0, pv
	jsr	ra, (pv)
END(_dl_start)

/*
 * Lazy binding entry point, called via PLT.
 */
NESTED_NOPROFILE(_dl_bind_start, 0, 168, ra, 0, 0)
	.set	noat
	/* at_reg already used by PLT code. */

	/*
	 * Allocate stack frame and preserve all registers that the caller
	 * would have normally saved themselves.
	 */
	lda	sp, -168(sp)
	stq	ra, 0(sp)
	stq	v0, 8(sp)
	stq	t0, 16(sp)
	stq	t1, 24(sp)
	stq	t2, 32(sp)
	stq	t3, 40(sp)
	stq	t4, 48(sp)
	stq	t5, 56(sp)
	stq	t6, 64(sp)
	stq	t7, 72(sp)
	stq	a0, 80(sp)
	stq	a1, 88(sp)
	stq	a2, 96(sp)
	stq	a3, 104(sp)
	stq	a4, 112(sp)
	stq	a5, 120(sp)
	stq	t8, 128(sp)
	stq	t9, 136(sp)
	stq	t10, 144(sp)
	stq	t11, 152(sp)
	stq	gp, 160(sp)

	/*
	 * Load our global pointer.  Note, can't use pv, since it is
	 * already used by the PLT code.
	 */
	br	t0, L100
L100:	LDGP(t0)

	/* Set up the arguments for _dl_bind. */
	subq	at_reg, t12, a1
	ldq	a0, 8(t12)
	subq	a1, 20, a1
	addq	a1, a1, a1
	CALL(_dl_bind)

	/* Move the destination address into position. */
	mov	v0, pv

	/* Restore program registers. */
	ldq	ra, 0(sp)
	ldq	v0, 8(sp)
	ldq	t0, 16(sp)
	ldq	t1, 24(sp)
	ldq	t2, 32(sp)
	ldq	t3, 40(sp)
	ldq	t4, 48(sp)
	ldq	t5, 56(sp)
	ldq	t6, 64(sp)
	ldq	t7, 72(sp)
	ldq	a0, 80(sp)
	ldq	a1, 88(sp)
	ldq	a2, 96(sp)
	ldq	a3, 104(sp)
	ldq	a4, 112(sp)
	ldq	a5, 120(sp)
	ldq	t8, 128(sp)
	ldq	t9, 136(sp)
	ldq	t10, 144(sp)
	ldq	t11, 152(sp)
	ldq	gp, 160(sp)
	/* XXX LDGP? */

	/*
	 * We've patched the PLT; sync the I-stream.
	 */
	imb

	/* Pop the stack frame and turn control to the destination. */
	lda	sp, 168(sp)
	jmp	zero, (pv)
END(_dl_bind_start)


/*
 * In reality these are not leaves, but they are stubs which does not need
 * further register saving.
 */

LEAF_NOPROFILE(_dl_exit, 1)
	ldiq	v0, SYS_exit
	call_pal PAL_OSF1_callsys
	RET
END(_dl_exit)

LEAF_NOPROFILE(_dl_open, 2)
	ldiq	v0, SYS_open
	call_pal PAL_OSF1_callsys
	beq	a3, _dl_open_no_error
	subq	zero, v0, v0
_dl_open_no_error:
	RET
END(_dl_open)

LEAF_NOPROFILE(_dl_close, 1)
	ldiq	v0, SYS_close
	call_pal PAL_OSF1_callsys
	RET
END(_dl_close)

LEAF_NOPROFILE(_dl_write, 3)
	ldiq	v0, SYS_write
	call_pal PAL_OSF1_callsys
	RET
END(_dl_write)

LEAF_NOPROFILE(_dl_read, 3)
	ldiq	v0, SYS_read
	call_pal PAL_OSF1_callsys
	RET
END(_dl_read)

LEAF_NOPROFILE(_dl_mmap, 6)
	lda	sp, -8(sp)
	stq	a5, 0(sp)
	ldiq	v0, SYS_mmap
	call_pal PAL_OSF1_callsys
	lda	sp, 8(sp)
	RET
END(_dl_mmap)

LEAF_NOPROFILE(_dl_munmap, 2)
	ldiq	v0, SYS_munmap
	call_pal PAL_OSF1_callsys
	RET
END(_dl_munmap)

LEAF_NOPROFILE(_dl_mprotect, 3)
	ldiq	v0, SYS_mprotect
	call_pal PAL_OSF1_callsys
	RET
END(_dl_mprotect)

LEAF_NOPROFILE(_dl_issetugid, 0)
	ldiq	v0, SYS_issetugid
	call_pal PAL_OSF1_callsys
	RET
END(_dl_issetugid)

LEAF_NOPROFILE(_dl_stat, 2)
	ldiq	v0, SYS_stat
	call_pal PAL_OSF1_callsys
	RET
END(_dl_stat)

LEAF_NOPROFILE(_dl__syscall, 3)
	ldiq	v0, SYS___syscall
	call_pal PAL_OSF1_callsys
	RET
END(_dl__syscall)

LEAF_NOPROFILE(_dl_fstat, 2)
	ldiq	v0, SYS_fstat
	call_pal PAL_OSF1_callsys
	RET
END(_dl_fstat)

LEAF_NOPROFILE(_dl_fcntl, 3)
	ldiq	v0, SYS_fcntl
	call_pal PAL_OSF1_callsys
	RET
END(_dl_fcntl)

LEAF_NOPROFILE(_dl_getdirentries, 4)
	ldiq	v0, SYS_getdirentries
	call_pal PAL_OSF1_callsys
	RET
END(_dl_getdirentries)

/* _dl_sigprocmask does not support NULL new mask */
LEAF_NOPROFILE(_dl_sigprocmask, 4)
	mov	a2, a5
	ldl	a1, 0(a1)		/* load the set from *set */
	ldiq	v0, SYS_sigprocmask
	call_pal PAL_OSF1_callsys
	/* What about syscalls failing? */
	beq	a5, 1f
	stl	v0, 0(a5)
1:	mov	zero, v0
	RET
END(_dl_sigprocmask)

LEAF_NOPROFILE(_dl_sysctl, 4)
	ldiq	v0, SYS___sysctl
	call_pal PAL_OSF1_callsys
	RET
END(_dl_sysctl)

LEAF_NOPROFILE(_dl_gettimeofday, 2)
	ldiq	v0, SYS_gettimeofday
	call_pal PAL_OSF1_callsys
	RET
END(_dl_gettimeofday)

