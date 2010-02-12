/*
 * Copyright (c) 1997 Matthew R. Green
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1988 University of Utah.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
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
 * from: Utah $Hdr: vm_mmap.c 1.6 91/10/21$
 *
 *	@(#)vm_mmap.c	8.5 (Berkeley) 5/19/94
 */

/*
 * Mapped file (mmap) interface to VM
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/resourcevar.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/conf.h>

#include <sys/swap.h>
#include <sys/mount.h>
#include <sys/syscallargs.h>

#include <miscfs/specfs/specdev.h>

#include <uvm/uvm_extern.h>

/* ARGSUSED */
int
compat_43_sys_getpagesize(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{

	*retval = PAGE_SIZE;
	return (0);
}

int
compat_43_sys_mmap(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_43_sys_mmap_args /* {
		syscallarg(caddr_t) addr;
		syscallarg(size_t) len;
		syscallarg(int) prot;
		syscallarg(int) flags;
		syscallarg(int) fd;
		syscallarg(long) pos;
	} */ *uap = v;
	struct sys_mmap_args /* {
		syscallarg(caddr_t) addr;
		syscallarg(size_t) len;
		syscallarg(int) prot;
		syscallarg(int) flags;
		syscallarg(int) fd;
		syscallarg(long) pad;
		syscallarg(off_t) pos;
	} */ nargs;
	static const char cvtbsdprot[8] = {
		0,
		PROT_EXEC,
		PROT_WRITE,
		PROT_EXEC|PROT_WRITE,
		PROT_READ,
		PROT_EXEC|PROT_READ,
		PROT_WRITE|PROT_READ,
		PROT_EXEC|PROT_WRITE|PROT_READ,
	};
#define	OMAP_ANON	0x0002
#define	OMAP_COPY	0x0020
#define	OMAP_SHARED	0x0010
#define	OMAP_FIXED	0x0100
#define	OMAP_INHERIT	0x0800

	SCARG(&nargs, addr) = SCARG(uap, addr);
	SCARG(&nargs, len) = SCARG(uap, len);
	SCARG(&nargs, prot) = cvtbsdprot[SCARG(uap, prot)&0x7];
	SCARG(&nargs, flags) = 0;
	if (SCARG(uap, flags) & OMAP_ANON)
		SCARG(&nargs, flags) |= MAP_ANON;
	if (SCARG(uap, flags) & OMAP_COPY)
		SCARG(&nargs, flags) |= MAP_PRIVATE;
	if (SCARG(uap, flags) & OMAP_SHARED)
		SCARG(&nargs, flags) |= MAP_SHARED;
	else
		SCARG(&nargs, flags) |= MAP_PRIVATE;
	if (SCARG(uap, flags) & OMAP_FIXED)
		SCARG(&nargs, flags) |= MAP_FIXED;
	if (SCARG(uap, flags) & OMAP_INHERIT)
		SCARG(&nargs, flags) |= MAP_INHERIT;
	SCARG(&nargs, fd) = SCARG(uap, fd);
	SCARG(&nargs, pos) = SCARG(uap, pos);
	return (sys_mmap(p, &nargs, retval));
}

int
compat_43_sys_swapon(struct proc *p, void *v, register_t *retval)
{
	struct sys_swapctl_args ua;
	struct compat_43_sys_swapon_args /* {
		syscallarg(const char *) name;
	} */ *uap = v;

	SCARG(&ua, cmd) = SWAP_ON;
	SCARG(&ua, arg) = (void *)SCARG(uap, name);
	SCARG(&ua, misc) = 0;	/* priority */
	return (sys_swapctl(p, &ua, retval));
}

int
compat_43_sys_obreak(struct proc *p, void *v, register_t *retval)
{
	struct compat_43_sys_obreak_args /* {
		syscallarg(char *) nsize;
	} */ *uap = v;
	struct vmspace *vm = p->p_vmspace;
	vaddr_t new, old;
	int error;

	old = (vaddr_t)vm->vm_daddr;
	new = round_page((vaddr_t)SCARG(uap, nsize));
	if ((new - old) > p->p_rlimit[RLIMIT_DATA].rlim_cur)
		return (ENOMEM);

	old = round_page(old + ptoa(vm->vm_dsize));

	if (new == old)
		return (0);

	/*
	 * grow or shrink?
	 */
	if (new > old) {
		error = uvm_map(&vm->vm_map, &old, new - old, NULL,
		    UVM_UNKNOWN_OFFSET, 0,
		    UVM_MAPFLAG(UVM_PROT_RW, UVM_PROT_RWX, UVM_INH_COPY,
		    UVM_ADV_NORMAL, UVM_FLAG_AMAPPAD|UVM_FLAG_FIXED|
		    UVM_FLAG_OVERLAY|UVM_FLAG_COPYONW));
		if (error) {
			uprintf("sbrk: grow %ld failed, error = %d\n",
			    new - old, error);
			return (ENOMEM);
		}
		vm->vm_dsize += atop(new - old);
	} else {
		uvm_deallocate(&vm->vm_map, new, old - new);
		vm->vm_dsize -= atop(old - new);
	}

	return (0);
}

/* ARGSUSED */
int
compat_43_sys_sbrk(struct proc *p, void *v, register_t *retval)
{
#if 0
	struct compat_43_sys_sbrk_args /* {
		syscallarg(int) incr;
	} */ *uap = v;
#endif

	return (ENOSYS);
}

/* ARGSUSED */
int
compat_43_sys_sstk(struct proc *p, void *v, register_t *retval)
{
#if 0
	struct compat_43_sys_sstk_args /* {
		syscallarg(int) incr;
	} */ *uap = v;
#endif

	return (ENOSYS);
}
