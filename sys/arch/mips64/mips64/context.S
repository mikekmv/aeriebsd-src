
/*
 * Copyright (c) 2002-2003 Opsycon AB  (www.opsycon.se / www.opsycon.com)
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
#include <sys/errno.h>
#include <sys/syscall.h>

#include <machine/param.h>
#include <machine/psl.h>
#include <machine/asm.h>
#include <machine/cpu.h>
#include <machine/regnum.h>
#include <machine/cpustate.h>

#include "assym.h"

#define curproc	(cpu_info_primary + CI_CURPROC)

	.set	mips3

	.set	noreorder		# Noreorder is default style!

/*
 * Save registers and state used by reboot to take snapshot.
 */
LEAF(savectx, 0)
	REG_S	s0, PCB_CONTEXT+0*REGSZ(a0)
	REG_S	s1, PCB_CONTEXT+1*REGSZ(a0)
	REG_S	s2, PCB_CONTEXT+2*REGSZ(a0)
	REG_S	s3, PCB_CONTEXT+3*REGSZ(a0)
	mfc0	v0, COP_0_STATUS_REG
	REG_S	s4, PCB_CONTEXT+4*REGSZ(a0)
	REG_S	s5, PCB_CONTEXT+5*REGSZ(a0)
	REG_S	s6, PCB_CONTEXT+6*REGSZ(a0)
	REG_S	s7, PCB_CONTEXT+7*REGSZ(a0)
	REG_S	sp, PCB_CONTEXT+8*REGSZ(a0)
	REG_S	s8, PCB_CONTEXT+9*REGSZ(a0)
	REG_S	ra, PCB_CONTEXT+10*REGSZ(a0)
	REG_S	v0, PCB_CONTEXT+11*REGSZ(a0)
	cfc0	t1, COP_0_ICR
	lw	t0, cpl
	REG_S	t1, PCB_CONTEXT+12*REGSZ(a0)	# save status register
	REG_S	t0, PCB_CONTEXT+13*REGSZ(a0)
	j	ra
	move	v0, zero
END(savectx)

LEAF(cpu_idle_enter, 0)
#if defined(TGT_CP7000) || defined(TGT_CP7000G)
	PTR_L	t2, misc_h		# if non zero, do Ocelot LEDs.
	beqz	t2, 1f
	li	t0, 0x40
	sb	t0, 0x0d(t2)
1:
#endif

	j	ra
	nop
END(cpu_idle_enter)

LEAF(cpu_idle_leave, 0)
#if defined(TGT_CP7000) || defined(TGT_CP7000G)
	PTR_L	t2, misc_h		# if non zero, do Ocelot LEDs.
	beqz	t2, 1f
	li	t0, 0x40
	sb	t0, 0x0c(t2)
1:
#endif

	j	ra
	nop
END(cpu_idle_leave)

LEAF(cpu_idle_cycle, 0)
	j	ra
	nop
END(cpu_idle_cycle)

/*
 * cpu_switchto(struct proc *oldproc, struct proc *newproc)
 */
NON_LEAF(cpu_switchto, FRAMESZ(CF_SZ), ra)
	PTR_L	t3, curprocpaddr
	REG_S	sp, PCB_CONTEXT+8*REGSZ(t3)	# save old sp

	PTR_SUBU sp, sp, FRAMESZ(CF_SZ)
	REG_S	ra, CF_RA_OFFS(sp)
	.mask	0x80000000, (CF_RA_OFFS - FRAMESZ(CF_SZ))

	beqz	a0, 1f
	mfc0	v0, COP_0_STATUS_REG

	lw	t0, cpl
	REG_S	s0, PCB_CONTEXT+0*REGSZ(t3)	# do a 'savectx()'
	REG_S	s1, PCB_CONTEXT+1*REGSZ(t3)
	REG_S	s2, PCB_CONTEXT+2*REGSZ(t3)
	REG_S	s3, PCB_CONTEXT+3*REGSZ(t3)
	REG_S	s4, PCB_CONTEXT+4*REGSZ(t3)
	REG_S	s5, PCB_CONTEXT+5*REGSZ(t3)
	REG_S	s6, PCB_CONTEXT+6*REGSZ(t3)
	REG_S	s7, PCB_CONTEXT+7*REGSZ(t3)
	REG_S	s8, PCB_CONTEXT+9*REGSZ(t3)
	REG_S	ra, PCB_CONTEXT+10*REGSZ(t3)
	cfc0	t1, COP_0_ICR
	REG_S	v0, PCB_CONTEXT+11*REGSZ(t3)
	REG_S	t1, PCB_CONTEXT+12*REGSZ(t3)
	REG_S	t0, PCB_CONTEXT+13*REGSZ(t3)

1:
	/*
	 * Disable interrupts
	 */
	li	v1, ~SR_INT_ENAB
	and	v0, v0, v1
	mtc0	v0, COP_0_STATUS_REG
	ITLBNOPFIX

	/*
	 * Switch to new context
 	*/
	move	s0, a1				# save p
	jal	pmap_activate
	move	a0, s0

	PTR_L	t3, P_ADDR(s0)			# get uarea pointer.
	PTR_S	s0, curproc			# set curproc
	PTR_S	t3, curprocpaddr

	li	t1, SONPROC
	sb	t1, P_STAT(s0)			# set to onproc.

	/*
	 * We need to wire the process kernel stack mapping so there
	 * will be no tlb misses in exception handlers. This is done
	 * by invalidating any tlb entries mapping the U-area and
	 * put valid mappings in tlb entries 0 and 1.
	 */

	/* get process ASID */
	PTR_L	t0, P_VMSPACE(s0)		# p->p_vmspace
	PTR_L	t1, VMSPACE_PMAP(t0)		# ->vm_map.pmap
	lw	v0, PM_TLBPID(t1)		# ->pm_tlbpid

	or	v0, t3
	dmtc0	v0, COP_0_TLB_HI		# init high entry (tlbid)
	LA	t1, (VM_MIN_KERNEL_ADDRESS)
	PTR_SUBU t2, t3, t1
	bltz	t2, ctx3			# not mapped.
	PTR_SRL	t2, PGSHIFT+1
	PTR_L	t1, Sysmap
	tlbp
	PTR_SLL	t2, 3
	PTR_ADDU t1, t2				# t1 now points at ptes.
	mfc0	t0, COP_0_TLB_INDEX
	nop
	bltz	t0, ctx1			# not in tlb
	LA	t2, KSEG0_BASE		# NOTE: if > 1 ins, does not matter

	dmtc0	t2, COP_0_TLB_HI		# invalidate it.
	dmtc0	zero, COP_0_TLB_LO0
	dmtc0	zero, COP_0_TLB_LO1
	nop
	nop
	nop
	nop
	tlbwi
	nop
	nop
	nop

ctx1:
	mtc0	zero, COP_0_TLB_INDEX
	dmtc0	v0, COP_0_TLB_HI
	lw	ta0, 0(t1)
	lw	ta1, 4(t1)
	dsll	ta0, ta0, 34
	dsrl	ta0, ta0, 34
	dsll	ta1, ta1, 34
	dsrl	ta1, ta1, 34
	dmtc0	ta0, COP_0_TLB_LO0
	dmtc0	ta1, COP_0_TLB_LO1
	nop
	PTR_ADDU v0, 2*NBPG
	nop
	nop
	tlbwi

#if (UPAGES != 2)
	dmtc0	v0, COP_0_TLB_HI		# init high entry (tlbid)
	lw	ta0, 8(t1)
	lw	ta1, 12(t1)
	dsll	ta0, ta0, 34
	dsrl	ta0, ta0, 34
	tlbp
	nop
	dsll	ta1, ta1, 34
	dsrl	ta1, ta1, 34
	mfc0	t0, COP_0_TLB_INDEX
	nop
	bltz	t0, ctx2			# not in tlb
	li	t2, 1

	dmtc0	t2, COP_0_TLB_HI		# invalidate it.
	dmtc0	zero, COP_0_TLB_LO0
	dmtc0	zero, COP_0_TLB_LO1
	nop
	nop
	nop
	nop
	tlbwi
	nop
	nop
	nop

ctx2:
	mtc0	t2, COP_0_TLB_INDEX
	dmtc0	v0, COP_0_TLB_HI
	dmtc0	ta0, COP_0_TLB_LO0
	dmtc0	ta1, COP_0_TLB_LO1
	nop
	nop
	nop
	nop
	tlbwi
#endif
	nop
	nop
	nop
	nop

ctx3:

	/*
	 * Restore registers and return.
	 */

	REG_L	a0, PCB_CONTEXT+13*REGSZ(t3)
	REG_L	s0, PCB_CONTEXT+0*REGSZ(t3)
	REG_L	s1, PCB_CONTEXT+1*REGSZ(t3)
	REG_L	s2, PCB_CONTEXT+2*REGSZ(t3)
	REG_L	s3, PCB_CONTEXT+3*REGSZ(t3)
	REG_L	s4, PCB_CONTEXT+4*REGSZ(t3)
	REG_L	s5, PCB_CONTEXT+5*REGSZ(t3)
	REG_L	s6, PCB_CONTEXT+6*REGSZ(t3)
	REG_L	s7, PCB_CONTEXT+7*REGSZ(t3)
	REG_L	sp, PCB_CONTEXT+8*REGSZ(t3)
	REG_L	s8, PCB_CONTEXT+9*REGSZ(t3)
	sw	a0, cpl
#ifdef IMASK_EXTERNAL
	jal	hw_setintrmask
	nop
#endif
	REG_L	ra, PCB_CONTEXT+10*REGSZ(t3)
	REG_L	v0, PCB_CONTEXT+11*REGSZ(t3)
	REG_L	v1, PCB_CONTEXT+12*REGSZ(t3)
#ifndef IMASK_EXTERNAL
	ctc0	v1, COP_0_ICR		# XXX RM7000
#endif
	mtc0	v0, COP_0_STATUS_REG
	ITLBNOPFIX
	j	ra
	nop
END(cpu_switchto)
