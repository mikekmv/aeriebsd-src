/*
 * Copyright (c) 1995 Frank van der Linden
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
 *      This product includes software developed for the NetBSD Project
 *      by Frank van der Linden
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
 *
 *	from: linux_file.c,v 1.3 1995/04/04 04:21:30 mycroft Exp
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/filedesc.h>
#include <sys/ioctl.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/malloc.h>

#include <sys/syscallargs.h>

#include <compat/common/compat_util.h>
#include <compat/openbsd/openbsd_syscallargs.h>

extern const char openbsd_emul_path[];

#define OPENBSD_CHECK_ALT_EXIST(p, sgp, path) \
    CHECK_ALT_EXIST(p, sgp, openbsd_emul_path, path)

#define OPENBSD_CHECK_ALT_CREAT(p, sgp, path) \
    CHECK_ALT_CREAT(p, sgp, openbsd_emul_path, path)

/*
 * XXX mount/umount have to return EPERM
 */

int
openbsd_sys_mount(struct proc *p, void *v, register_t *retval)
{
	return sys_mount(p, v, retval);
}

int
openbsd_sys_unmount(struct proc *p, void *v, register_t *retval)
{
	return sys_unmount(p, v, retval);
}

/*
 * The following syscalls are only here because of the alternate path check.
 */

int
openbsd_sys_open(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_open_args /* {
		syscallarg(char *) path;
		syscallarg(int) flags;
		syscallarg(int) mode;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	if (SCARG(uap, flags) & O_CREAT)
		OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, path));
	else
		OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_open(p, uap, retval);
}

int
openbsd_sys_link(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_link_args /* {
		syscallarg(char *) path;
		syscallarg(char *) link;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, link));
	return sys_link(p, uap, retval);
}

int
openbsd_sys_unlink(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_unlink_args /* {
		syscallarg(char *) path;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_unlink(p, uap, retval);
}

int
openbsd_sys_chdir(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_chdir_args /* {
		syscallarg(char *) path;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_chdir(p, uap, retval);
}

int
openbsd_sys_mknod(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_mknod_args /* {
		syscallarg(char *) path;
		syscallarg(int) mode;
		syscallarg(int) dev;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, path));
	return sys_mknod(p, uap, retval);
}

int
openbsd_sys_chmod(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_chmod_args /* {
		syscallarg(char *) path;
		syscallarg(int) mode;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_chmod(p, uap, retval);
}

int
openbsd_sys_chown(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_chown_args /* {
		syscallarg(char *) path;
		syscallarg(int) uid;
		syscallarg(int) gid;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_chown(p, uap, retval);
}

int
openbsd_sys_lchown(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_chown_args /* {
		syscallarg(char *) path;
		syscallarg(int) uid;
		syscallarg(int) gid;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_lchown(p, uap, retval);
}

int
openbsd_sys_truncate(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_truncate_args /* {
		syscallarg(char *) path;
		syscallarg(int) pad;
		syscallarg(off_t) length;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_truncate(p, uap, retval);
}

int
openbsd_sys_chflags(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_chflags_args /* {
		syscallarg(char *) path;
		syscallarg(int) flags;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_chflags(p, uap, retval);
}

int
openbsd_sys_utimes(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_truncate_args /* {
		syscallarg(char *) path;
		syscallarg(const struct timeval *) tptr;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_utimes(p, uap, retval);
}

int
openbsd_sys_revoke(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_revoke_args /* {
		syscallarg(char *) path;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_revoke(p, uap, retval);
}

int
openbsd_sys_symlink(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_symlink_args /* {
		syscallarg(char *) path;
		syscallarg(char *) link;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, link));
	return sys_symlink(p, uap, retval);
}

int
openbsd_sys_readlink(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_readlink_args /* {
		syscallarg(char *) path;
		syscallarg(char *) buf;
		syscallarg(int) count;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_readlink(p, uap, retval);
}

int
openbsd_sys_execve(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_execve_args /* {
		syscallarg(char *) path;
		syscallarg(char **) argp;
		syscallarg(char **) envp;
	} */ *uap = v;
	struct sys_execve_args ap;
	caddr_t sg;

	sg = stackgap_init(p->p_emul);
	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));

	SCARG(&ap, path) = SCARG(uap, path);
	SCARG(&ap, argp) = SCARG(uap, argp);
	SCARG(&ap, envp) = SCARG(uap, envp);

	return sys_execve(p, &ap, retval);
}

int
openbsd_sys_chroot(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_chroot_args /* {
		syscallarg(char *) path;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_chroot(p, uap, retval);
}

int
openbsd_sys_rename(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_rename_args /* {
		syscallarg(char *) from;
		syscallarg(char *) to;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, from));
	OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, to));
	return sys_rename(p, uap, retval);
}

int
openbsd_sys_mkfifo(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_mkfifo_args /* {
		syscallarg(char *) path;
		syscallarg(int) mode;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, path));
	return sys_mkfifo(p, uap, retval);
}

int
openbsd_sys_mkdir(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_mkdir_args /* {
		syscallarg(char *) path;
		syscallarg(int) mode;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_CREAT(p, &sg, SCARG(uap, path));
	return sys_mkdir(p, uap, retval);
}

int
openbsd_sys_rmdir(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_rmdir_args /* {
		syscallarg(char *) path;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_rmdir(p, uap, retval);
}

#ifdef NFSCLIENT
int
openbsd_sys_getfh(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_getfh_args /* {
		syscallarg(char *) fname;
		syscallarg(fhandle_t *) fhp;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, fname));
	return sys_getfh(p, uap, retval);
}
#endif /* NFSCLIENT */

int
openbsd_sys_access(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_access_args /* {
		syscallarg(char *) path;
		syscallarg(int) flags;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_access(p, uap, retval);
}

int
openbsd_sys_stat(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_stat_args /* {
		syscallarg(char *) path;
		syscallarg(struct stat *) ub;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_stat(p, uap, retval);
}

int
openbsd_sys_lstat(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_lstat_args /* {
		syscallarg(char *) path;
		syscallarg(struct stat *) ub;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_lstat(p, uap, retval);
}

int
openbsd_sys_pathconf(struct proc *p, void *v, register_t *retval)
{
	struct openbsd_sys_pathconf_args /* {
		syscallarg(char *) path;
		syscallarg(int) name;
	} */ *uap = v;
	caddr_t sg = stackgap_init(p->p_emul);

	OPENBSD_CHECK_ALT_EXIST(p, &sg, SCARG(uap, path));
	return sys_pathconf(p, uap, retval);
}
