/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)vfs_syscalls.c	8.28 (Berkeley) 12/10/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/dirent.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <sys/syscallargs.h>

void statfs_to_ostatfs(struct proc *, struct mount *, struct statfs *, struct ostatfs *);

/*
 * Convert struct statfs -> struct ostatfs
 */
void
statfs_to_ostatfs(p, mp, sp, osp)
	struct proc *p;
	struct mount *mp;
	struct statfs *sp;
	struct ostatfs *osp;
{
#ifdef COMPAT_43
	osp->f_type = mp->mnt_vfc->vfc_typenum;
#else
	osp->f_type = 0;
#endif
	osp->f_flags = mp->mnt_flag & 0xffff;
	osp->f_bsize = sp->f_bsize;
	osp->f_iosize = sp->f_iosize;
	osp->f_blocks = sp->f_blocks;
	osp->f_bfree = sp->f_bfree;
	osp->f_bavail = sp->f_bavail;
	osp->f_files = sp->f_files;
	osp->f_ffree = sp->f_ffree;
	/* Don't let non-root see filesystem id (for NFS security) */
	if (suser(p, 0))
		osp->f_fsid.val[0] = osp->f_fsid.val[1] = 0;
	else
		bcopy(&sp->f_fsid, &osp->f_fsid, sizeof(osp->f_fsid));
	osp->f_owner = sp->f_owner;
	osp->f_syncwrites = sp->f_syncwrites;
	osp->f_asyncwrites = sp->f_asyncwrites;
	bcopy(sp->f_fstypename, osp->f_fstypename, MFSNAMELEN);
	bcopy(sp->f_mntonname, osp->f_mntonname, MNAMELEN);
	bcopy(sp->f_mntfromname, osp->f_mntfromname, MNAMELEN);
}

/*
 * Get filesystem statistics.
 */
/* ARGSUSED */
int
compat_44_sys_statfs(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_44_sys_statfs_args /* {
		syscallarg(char *) path;
		syscallarg(struct ostatfs *) buf;
	} */ *uap = v;
	register struct mount *mp;
	register struct statfs *sp;
	struct ostatfs osb;
	int error;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, FOLLOW, UIO_USERSPACE, SCARG(uap, path), p);
	if ((error = namei(&nd)) != 0)
		return (error);
	mp = nd.ni_vp->v_mount;
	sp = &mp->mnt_stat;
	vrele(nd.ni_vp);
	if ((error = VFS_STATFS(mp, sp, p)) != 0)
		return (error);

	statfs_to_ostatfs(p, mp, sp, &osb);
	return (copyout((caddr_t)&osb, (caddr_t)SCARG(uap, buf), sizeof(osb)));
}

/*
 * Get filesystem statistics.
 */
/* ARGSUSED */
int
compat_44_sys_fstatfs(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct compat_44_sys_fstatfs_args /* {
		syscallarg(int) fd;
		syscallarg(struct ostatfs *) buf;
	} */ *uap = v;
	struct file *fp;
	struct mount *mp;
	struct statfs *sp;
	struct ostatfs osb;
	int error;

	if ((error = getvnode(p->p_fd, SCARG(uap, fd), &fp)) != 0)
		return (error);
	mp = ((struct vnode *)fp->f_data)->v_mount;
	sp = &mp->mnt_stat;
	error = VFS_STATFS(mp, sp, p);
	FRELE(fp);
	if (error)
		return (error);

	statfs_to_ostatfs(p, mp, sp, &osb);
	return (copyout((caddr_t)&osb, (caddr_t)SCARG(uap, buf), sizeof(osb)));
}

/*
 * Get statistics on all filesystems.
 */
int
compat_44_sys_getfsstat(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	register struct compat_44_sys_getfsstat_args /* {
		syscallarg(struct ostatfs *) buf;
		syscallarg(long) bufsize;
		syscallarg(int) flags;
	} */ *uap = v;
	register struct mount *mp, *nmp;
	register struct statfs *sp;
	struct ostatfs osb;
	caddr_t sfsp;
	long count, maxcount;
	int error, flags = SCARG(uap, flags);

	maxcount = SCARG(uap, bufsize) / sizeof(struct ostatfs);
	sfsp = (caddr_t)SCARG(uap, buf);
	count = 0;

	for (mp = CIRCLEQ_FIRST(&mountlist); mp != CIRCLEQ_END(&mountlist);
	    mp = nmp) {
		if (vfs_busy(mp, VB_READ|VB_NOWAIT)) {
			nmp = CIRCLEQ_NEXT(mp, mnt_list);
			continue;
		}
		if (sfsp && count < maxcount) {
			sp = &mp->mnt_stat;

			/* Refresh stats unless MNT_NOWAIT is specified */
			if (flags != MNT_NOWAIT &&
			    flags != MNT_LAZY &&
			    (flags == MNT_WAIT ||
			     flags == 0) &&
			    (error = VFS_STATFS(mp, sp, p))) {
				nmp = CIRCLEQ_NEXT(mp, mnt_list);
				vfs_unbusy(mp);
 				continue;
			}

			statfs_to_ostatfs(p, mp, sp, &osb);
			error = copyout((caddr_t)&osb, sfsp, sizeof(osb));
			if (error) {
				vfs_unbusy(mp);
				return (error);
			}
			sfsp += sizeof(osb);
		}
		count++;
		nmp = CIRCLEQ_NEXT(mp, mnt_list);
		vfs_unbusy(mp);
	}

	if (sfsp && count > maxcount)
		*retval = maxcount;
	else
		*retval = count;

	return (0);
}

static void cvtstat(struct stat *, struct stat44 *);

/*
 * Convert from a new to an old stat structure.
 */
static void
cvtstat(struct stat *st, struct stat44 *ost)
{

	ost->st_dev = st->st_dev;
	ost->st_ino = st->st_ino;
	ost->st_mode = st->st_mode & 0xffff;
	ost->st_nlink = st->st_nlink & 0xffff;
	ost->st_uid = st->st_uid;
	ost->st_gid = st->st_gid;
	ost->st_rdev = st->st_rdev;
	ost->st_atimespec = st->st_atimespec;
	ost->st_mtimespec = st->st_mtimespec;
	ost->st_ctimespec = st->st_ctimespec;
	ost->st_size = st->st_size;
	ost->st_blocks = st->st_blocks;
	ost->st_blksize = st->st_blksize;
	ost->st_flags = st->st_flags;
	ost->st_gen = st->st_gen;
}

/*
 * Get file status; this version follows links.
 */
/* ARGSUSED */
int
compat_44_sys_stat(struct proc *p, void *v, register_t *retval)
{
	struct compat_44_sys_stat_args /* {
		syscallarg(char *) path;
		syscallarg(struct stat44 *) ub;
	} */ *uap = v;
	struct stat sb;
	struct stat44 osb;
	int error;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF, UIO_USERSPACE,
	    SCARG(uap, path), p);
	if ((error = namei(&nd)) != 0)
		return (error);
	error = vn_stat(nd.ni_vp, &sb, p);
	vput(nd.ni_vp);
	if (error)
		return (error);
	/* Don't let non-root see generation numbers (for NFS security) */
	if (suser(p, 0))
		sb.st_gen = 0;
	cvtstat(&sb, &osb);
	error = copyout(&osb, SCARG(uap, ub), sizeof(osb));
	return (error);
}

/*
 * Get file status; this version does not follow links.
 */
/* ARGSUSED */
int
compat_44_sys_lstat(struct proc *p, void *v, register_t *retval)
{
	struct compat_44_sys_lstat_args /* {
		syscallarg(char *) path;
		syscallarg(struct stat44 *) ub;
	} */ *uap = v;
	struct stat sb;
	struct stat44 osb;
	int error;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, NOFOLLOW | LOCKLEAF, UIO_USERSPACE,
	    SCARG(uap, path), p);
	if ((error = namei(&nd)) != 0)
		return (error);
	error = vn_stat(nd.ni_vp, &sb, p);
	vput(nd.ni_vp);
	if (error)
		return (error);
	/* Don't let non-root see generation numbers (for NFS security) */
	if (suser(p, 0))
		sb.st_gen = 0;
	cvtstat(&sb, &osb);
	error = copyout(&osb, SCARG(uap, ub), sizeof(osb));
	return (error);
}

/*
 * Return status information about a file descriptor.
 */
/* ARGSUSED */
int
compat_44_sys_fstat(struct proc *p, void *v, register_t *retval)
{
	struct compat_44_sys_fstat_args /* {
		syscallarg(int) fd;
		syscallarg(struct stat44 *) sb;
	} */ *uap = v;
	int fd = SCARG(uap, fd);
	struct filedesc *fdp = p->p_fd;
	struct file *fp;
	struct stat ub;
	struct stat44 oub;
	int error;

	if ((fp = fd_getfile(fdp, fd)) == NULL)
		return (EBADF);
	FREF(fp);
	error = (*fp->f_ops->fo_stat)(fp, &ub, p);
	FRELE(fp);
	if (error == 0) {
		/* Don't let non-root see generation numbers
		   (for NFS security) */
		if (suser(p, 0))
			ub.st_gen = 0;
		cvtstat(&ub, &oub);
		error = copyout(&oub, SCARG(uap, sb), sizeof(oub));
	}
	return (error);
}

/* ARGSUSED */
int
compat_44_sys_fhstat(struct proc *p, void *v, register_t *retval)
{
	struct sys_fhstat_args /* {
		syscallarg(const fhandle_t *) fhp;
		syscallarg(struct stat44 *) sb;
	} */ *uap = v;
	struct stat ub;
	struct stat44 oub;
	int error;
	fhandle_t fh;
	struct mount *mp;
	struct vnode *vp;

	/*
	 * Must be super user
	 */
	if ((error = suser(p, 0)))
		return (error);

	if ((error = copyin(SCARG(uap, fhp), &fh, sizeof(fhandle_t))) != 0)
		return (error);

	if ((mp = vfs_getvfs(&fh.fh_fsid)) == NULL)
		return (ESTALE);
	if ((error = VFS_FHTOVP(mp, &fh.fh_fid, &vp)))
		return (error);
	error = vn_stat(vp, &ub, p);
	vput(vp);
	if (error)
		return (error);
	cvtstat(&ub, &oub);
	error = copyout(&oub, SCARG(uap, sb), sizeof(oub));
	return (error);
}
