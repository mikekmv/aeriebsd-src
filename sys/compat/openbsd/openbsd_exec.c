/*
 * Copyright (c) 1993, 1994 Christopher G. Demetriou
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/vnode.h>
#include <sys/exec.h>
#include <sys/signalvar.h>
#include <uvm/uvm_extern.h>
#include <sys/exec_elf.h>

#include <compat/common/compat_util.h>

#include <compat/openbsd/openbsd_syscall.h>
#include <compat/openbsd/openbsd_exec.h>
#include <machine/openbsd_machdep.h>

extern const struct sysent openbsd_sysent[];
#ifdef SYSCALL_DEBUG
extern const char *const openbsd_syscallnames[];
#endif
extern char sigcode[], esigcode[];

const char openbsd_emul_path[] = "/emul/openbsd";

#ifdef _KERN_DO_AOUT
struct emul emul_openbsd_aout = {
	"openbsd",
	NULL,
	sendsig,
	OPENBSD_SYS_syscall,
	OPENBSD_SYS_MAXSYSCALL,
	openbsd_sysent,
#ifdef SYSCALL_DEBUG
	openbsd_syscallnames,
#else
	NULL,
#endif
	0,
	copyargs,
	setregs,
	NULL,
	openbsd_sigcode,
	openbsd_esigcode,
	EMUL_ENABLED	/* XXX for now always enabled */
};
#endif

#ifdef _KERN_DO_ELF
struct emul emul_openbsd_elf32 = {
	"openbsd",
	NULL,
	sendsig,
	OPENBSD_SYS_syscall,
	OPENBSD_SYS_MAXSYSCALL,
	openbsd_sysent,
#ifdef SYSCALL_DEBUG
	openbsd_syscallnames,
#else
	NULL,
#endif
	sizeof (AuxInfo) * ELF_AUX_ENTRIES,
	elf32_copyargs,
	setregs,
	exec_elf32_fixup,
	openbsd_sigcode,
	openbsd_esigcode,
	EMUL_ENABLED	/* XXX for now always enabled */
};
#endif

#ifdef _KERN_DO_ELF64
struct emul emul_openbsd_elf64 = {
	"openbsd",
	NULL,
	sendsig,
	OPENBSD_SYS_syscall,
	OPENBSD_SYS_MAXSYSCALL,
	openbsd_sysent,
#ifdef SYSCALL_DEBUG
	openbsd_syscallnames,
#else
	NULL,
#endif
	sizeof (AuxInfo) * ELF_AUX_ENTRIES,
	elf64_copyargs,
	setregs,
	exec_elf64_fixup,
	openbsd_sigcode,
	openbsd_esigcode,
	EMUL_ENABLED	/* XXX for now always enabled */
};
#endif

#ifdef _KERN_DO_AOUT
/*
 * exec_aout_makecmds(): Check if it's an a.out-format executable.
 *
 * Given a proc pointer and an exec package pointer, see if the referent
 * of the epp is in a.out format.  First check 'standard' magic numbers for
 * this architecture.  If that fails, try a cpu-dependent hook.
 *
 * This function, in the former case, or the hook, in the latter, is
 * responsible for creating a set of vmcmds which can be used to build
 * the process's vm space and inserting them into the exec package.
 */

int
exec_openbsd_aout_makecmds(p, epp)
	struct proc *p;
	struct exec_package *epp;
{
	u_long midmag;
	int error = ENOEXEC;
	struct exec *execp = epp->ep_hdr;

	if (epp->ep_hdrvalid < sizeof(struct exec))
		return ENOEXEC;

	midmag = OPENBSD_N_GETMID(*execp) << 16 | OPENBSD_N_GETMAGIC(*execp);

	/* assume FreeBSD's MID_MACHINE and [ZQNO]MAGIC is same as NetBSD's */
	switch (midmag) {
	case (MID_MACHINE << 16) | ZMAGIC:
		error = exec_aout_prep_oldzmagic(p, epp);
		break;
	case (MID_MACHINE << 16) | QMAGIC:
		error = exec_aout_prep_zmagic(p, epp);
		break;
	case (MID_MACHINE << 16) | NMAGIC:
		error = exec_aout_prep_nmagic(p, epp);
		break;
	case (MID_MACHINE << 16) | OMAGIC:
		error = exec_aout_prep_omagic(p, epp);
		break;
	}
	if (error == 0)
		epp->ep_emul = &emul_openbsd_aout;
	else
		kill_vmcmds(&epp->ep_vmcmds);

	return error;
}
#endif

#ifdef _KERN_DO_ELF
int
openbsd_elf32_makecmds(struct proc *p, struct exec_package *epp)
{
	if (!(emul_openbsd_elf32.e_flags & EMUL_ENABLED))
		return (ENOEXEC);
	return exec_elf32_makecmds(p, epp);
}

int
openbsd_elf32_probe(struct proc *p, struct exec_package *epp, char *itp,
    u_long *pos, u_int8_t *os)
{
	char *bp;
	size_t len;
	int error;

	if (!(emul_openbsd_elf32.e_flags & EMUL_ENABLED))
		return (ENOEXEC);

	if (elf32_os_pt_note(p, epp, epp->ep_hdr, "OpenBSD", 8, 4))
		return (EINVAL);

	if (itp) {
		if ((error = emul_find(p, NULL, openbsd_emul_path, itp, &bp, 0)))
			return (error);
		if ((error = copystr(bp, itp, MAXPATHLEN, &len)))
			return (error);
		free(bp, M_TEMP);
	}
	epp->ep_emul = &emul_openbsd_elf32;
	*pos = ELF32_NO_ADDR;
	return (0);
}
#endif

#ifdef _KERN_DO_ELF64
int
openbsd_elf64_makecmds(struct proc *p, struct exec_package *epp)
{
	if (!(emul_openbsd_elf64.e_flags & EMUL_ENABLED))
		return (ENOEXEC);
	return exec_elf64_makecmds(p, epp);
}

int
openbsd_elf64_probe(struct proc *p, struct exec_package *epp, char *itp,
    u_long *pos, u_int8_t *os)
{
	char *bp;
	size_t len;
	int error;

	if (!(emul_openbsd_elf64.e_flags & EMUL_ENABLED))
		return (ENOEXEC);

	if (elf64_os_pt_note(p, epp, epp->ep_hdr, "OpenBSD", 8, 4))
		return (EINVAL);

	if (itp) {
		if ((error = emul_find(p, NULL, openbsd_emul_path, itp, &bp, 0)))
			return (error);
		if ((error = copystr(bp, itp, MAXPATHLEN, &len)))
			return (error);
		free(bp, M_TEMP);
	}
	epp->ep_emul = &emul_openbsd_elf64;
	*pos = ELF64_NO_ADDR;
	return (0);
}
#endif
