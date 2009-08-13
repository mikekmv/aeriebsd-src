/*
 * Copyright (c) 2009 Michael Shalayeff
 * Copyright (c) 2009 Pedro Martelletto
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/disklabel.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/pool.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/mount.h>

#include <milfs/milfs.h>
#include <milfs/milfs_extern.h>

struct pool milfs_block_pool;
struct pool milfs_inode_pool;

const struct vfsops milfs_vfsops = {
	milfs_mount,	/* mount */
	milfs_start,	/* start */
	milfs_unmount,	/* unmount */
	milfs_root,	/* root */
	milfs_quotactl,	/* quotactl */
	milfs_statfs,	/* statfs */
	milfs_sync,	/* sync */
	milfs_vget,	/* vget */
	milfs_fhtovp,	/* fhtovp */
	milfs_vptofh,	/* vptofh */
	milfs_init,	/* init */
	milfs_sysctl,	/* sysctl */
	milfs_checkexp,	/* checkexp */
};

/*
 * Check arguments, open the device, call milfs_mountfs() to do the real work.
 */
int
milfs_mount(struct mount *mp, const char *path, void *data,
    struct nameidata *ndp, struct proc *p)
{
	int close_error, filmode, error;
	mode_t accmode;
	size_t len;
	struct milfs_args args;
	struct vnode *vp;

#ifdef DIAGNOSTIC
	/*
	 * Many subroutines make this assumption, so don't take any risks.
	 */
	if (p != curproc)
		panic("milfs_mount: p is not curproc, %p != %p", p, curproc);
#endif

	/*
	 * Verify mount arguments.
	 */

	if ((mp->mnt_flag & MNT_LOCAL) == 0)
		return (EINVAL);

	if (mp->mnt_flag & ~(MNT_LOCAL|MNT_RDONLY))
		return (EINVAL);

	error = copyin(data, &args, sizeof(struct milfs_args));
	if (error)
		return (error);

	NDINIT(ndp, LOOKUP, FOLLOW, UIO_USERSPACE, args.fspec, p);
	error = namei(ndp);
	if (error)
		return (error);

	vp = ndp->ni_vp;

	/*
	 * We now hold a reference to the device vnode, 'vp'.
	 */

	if (vp->v_type != VBLK) {
		vrele(vp);
		return (ENOTBLK);
	}

	error = vfs_mountedon(vp);
	if (error) {
		vrele(vp);
		return (error);
	}

	if (rootvp != vp && vcount(vp) > 1) {
		vrele(vp);
		return (EBUSY);
	}

	/*
	 * See if we have permissions on the device.
	 */

	accmode = VREAD;
	filmode = FREAD;

	if ((mp->mnt_flag & MNT_RDONLY) == 0) {
		accmode |= VWRITE;
		filmode |= FWRITE;
	}

	vn_lock(vp, LK_EXCLUSIVE | LK_RETRY, p);

	/*
	 * 'vp' is now locked.
	 */

	if (suser(p, 0)) {
		error = VOP_ACCESS(vp, accmode, p->p_ucred, p);
		if (error) {
			vput(vp);
			return (error);
		}
	}

	/* Flush all buffers currently associated with the device. */
	error = vinvalbuf(vp, V_SAVE, p->p_ucred, p, 0, 0);
	if (error) {
		vput(vp);
		return (error);
	}

	VOP_UNLOCK(vp, 0, p);

	/*
	 * 'vp' is now unlocked.
	 */

	error = VOP_OPEN(vp, filmode, p->p_ucred, p);
	if (error) {
		vrele(vp);
		return (error);
	}

	error = milfs_mountfs(mp, vp);
	if (error) {
		vn_lock(vp, LK_EXCLUSIVE | LK_RETRY, p);
		close_error = VOP_CLOSE(vp, filmode, p->p_ucred, p);
		if (close_error)
			panic("milfs_mount: could not close %p, error %d",
			    vp, close_error);
		vput(vp);
	} else {
		copyinstr(path, mp->mnt_stat.f_mntonname, MNAMELEN - 1, &len);
		bzero(mp->mnt_stat.f_mntonname + len, MNAMELEN - len);
		copyinstr(args.fspec, mp->mnt_stat.f_mntfromname, MNAMELEN - 1,
		    &len);
		bzero(mp->mnt_stat.f_mntfromname + len, MNAMELEN - len);
	}

	return (error);
}

/*
 * Called for every successful milfs_mount().
 */
int
milfs_start(struct mount *mp, int flags, struct proc *p)
{
	printf("milfs_start called!\n");
	return (0);
}

int
milfs_unmount(struct mount *mp, int flags, struct proc *p)
{
	struct milfs_mount *mmp = VFSTOMILFS(mp);

	mmp->mm_flags |= MILFS_GONE;

	printf("milfs_unmount called!\n");

	/* wait for the cleaner to finish */
	if (mmp->mm_cleaner) {
		wakeup(mmp->mm_cleaner);
		tsleep(mmp, PRIBIO, "kaput", 0);
	}

	return (EINVAL);
}

int
milfs_root(struct mount *mp, struct vnode **vpp)
{
	int error;
	struct vnode *vp;

	error = VFS_VGET(mp, MILFS_INODE_ROOT, &vp);
	if (error)
		return (error);

	*vpp = vp;

	return (0);
}

int milfs_quotactl (struct mount *mp, int cmd, uid_t uid, caddr_t arg,
                    struct proc *p) {
  return EOPNOTSUPP;
}

int milfs_statfs(struct mount *mp, struct statfs *sbp, struct proc *p) {
  printf("milfs_statfs called!\n");
  return (EINVAL);
}

int milfs_sync(struct mount *mp, int waitfor, struct ucred *crd,
  struct proc *p) {
  printf("milfs_sync called!\n");
    return (EINVAL);
}

int
milfs_vget(struct mount *mp, ino_t ino, struct vnode **vpp)
{
	int error;
	struct milfs_inode *mip;
	struct milfs_mount *mmp;
	struct proc *p;
	struct vnode *vp;

	/*
	 * See if the inode is known.
	 */
	mmp = (struct milfs_mount *)mp->mnt_data;
	mip = milfs_inolookup(mmp, ino);
	if (mip == NULL) {
		/*
		 * Inode is unknown. Try to find it in the inode directory.
		 * This will also get us the static information, as a bonus.
		 */
		mip = milfs_inodirlookup(mmp, NULL, ino);
		if (mip == NULL)
			panic("milfs_vget: missing entry");
	}

	/*
	 * If there is an associated vnode, try to get it.
	 */
	if (mip->mi_vnode != NULL) {
		p = curproc;
		error = vget(mip->mi_vnode, LK_EXCLUSIVE, p);
		if (error) {
#ifdef DIAGNOSTIC
			if (error == ENOENT)
				printf("milfs_vget: got ENOENT\n");
#endif
			*vpp = NULL;
			return (error);
		}

#ifdef DIAGNOSTIC
		if ((mip->mi_mode & MILFS_MODE_HASSTATIC) == 0)
			panic("milfs_vget: inode with vnode but no sinode");
#endif

		*vpp = mip->mi_vnode;

		return (0);
	}

	/*
	 * If we still haven't read the static inode entry, do it now.
	 */
	if ((mip->mi_mode & MILFS_MODE_HASSTATIC) == 0)
		mip = milfs_inodirlookup(mmp, mip, 0);

	/*
	 * No cached vnode, allocate new one.
	 */

	error = getnewvnode(VT_MILFS, mp, milfs_vnodeop_p, &vp);
	if (error) {
		*vpp = NULL;
		return (error);
	}

	vref(mmp->mm_devvp);

	if (mip->mi_mode & MILFS_MODE_DIRECTORY)
		vp->v_type = VDIR;
	else
		vp->v_type = VREG;

	if (mip->mi_inode == MILFS_INODE_ROOT)
		vp->v_flag |= VROOT;

	mip->mi_vnode = vp;
	vp->v_data = mip;

	*vpp = vp;

	return (0);
}

int milfs_fhtovp (struct mount *mp, struct fid *fhp, struct vnode **vpp) {
  return EOPNOTSUPP;
}


int milfs_vptofh (struct vnode *vp, struct fid *fhp) {
  return EOPNOTSUPP;
}

/*
 * Called once upon VFS initialization.
 */
int
milfs_init(struct vfsconf *vfsp)
{
	pool_init(&milfs_block_pool, sizeof(struct milfs_block), 0, 0, 0,
	    "milblkpl", &pool_allocator_nointr);
	pool_init(&milfs_inode_pool, sizeof(struct milfs_inode), 0, 0, 0,
	    "milinopl", &pool_allocator_nointr);

	return (0);
}

int milfs_sysctl (int *name, u_int namelen, void *oldp, size_t *oldlenp,
                  void *newp, size_t newlen, struct proc *p) {
  return EOPNOTSUPP;
}

int
milfs_checkexp(struct mount *mp, struct mbuf *nam, int *exflagsp,
    struct ucred **credanonp)
{
	return (EOPNOTSUPP);
}
