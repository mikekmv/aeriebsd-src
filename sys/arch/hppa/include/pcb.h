
/*
 * Copyright (c) 1999-2004 Michael Shalayeff
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _MACHINE_PCB_H_
#define _MACHINE_PCB_H_

#include <machine/reg.h>

struct pcb {
	u_int64_t pcb_fpregs[HPPA_NFPREGS+1];	/* not in the trapframe */
	vaddr_t		pcb_uva;	/* KVA for U-area */

	/*
	 * The members above are primarily accessed by there physical
	 * address, wheras the members below are accessed exclusively
	 * by there virtual address.  Unfortunately this structure
	 * ends up being non-equivalently mapped, which will cause
	 * data corruption if those members share a cache line.  Since
	 * the maximum cache line size is 64 bytes, adding 64 bytes of
	 * padding makes sure that will never happen.
	 */
	u_char		pcb_pad[64];

	u_int		pcb_ksp;	/* kernel sp for ctxsw */
	u_int		pcb_onfault;	/* SW copy fault handler */
	pa_space_t	pcb_space;	/* copy pmap_space, for asm's sake */

	/* things used for hpux emulation */
	void		*pcb_sigreturn;
	u_int		pcb_srcookie;
	u_int		pcb_sclen;
};

struct md_coredump {
	struct reg md_reg;
	struct fpreg md_fpreg;
}; 


#endif /* _MACHINE_PCB_H_ */
