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

#include <milfs/milfs_param.h>
#include <milfs/milfs_types.h>
#include <milfs/milfs_proto.h>
#include <milfs/milfs_extern.h>

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
	printf("milfs_unmount called!\n");
	return (EINVAL);
}

int
milfs_root(struct mount *mp, struct vnode **vpp)
{
	printf("milfs_root called!\n");
	return (EINVAL);
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

int milfs_vget (struct mount *mp, ino_t ino, struct vnode **vpp) {
  struct timeval tv;
  getmicrotime(&tv);
  printf("%d, %d\n", tv.tv_sec, tv.tv_usec);
  return EINVAL;
}

#if 0
/* Return a locked vnode corresponding to the inode 'ino'. */
int
milfs_vget(struct mount *mp, ino_t ino, struct vnode **vpp)
{
	int error;
	struct milfs_vnode *mvp;
	struct vnode *vp;

	*vpp = NULL;

	error = milfs_vhash_get(mp->mnt_data, ino, LK_EXCLUSIVE, curproc, vpp);
	if (error)
		return (error);

	if (*vpp != NULL)
		return (0);	/* Found in cache. */

	error = getnewvnode(VT_MILFS, mp, milfs_vnodeop_p, &vp);
	if (error) {
		*vpp = NULL;
		return (error);
	}

	mvp = pool_get(&milfs_vnode_pool, PR_WAITOK|PR_ZERO);

	printf("vp = %p, mvp = %p\n", vp, mvp);

	milfs_vhash_put(mvp, curproc);

	return (EINVAL);
}
#endif

int milfs_fhtovp (struct mount *mp, struct fid *fhp, struct vnode **vpp) {
  return EOPNOTSUPP;
}


int milfs_vptofh (struct vnode *vp, struct fid *fhp) {
  return EOPNOTSUPP;
}

/*
** called once upon vfs initialization
*/
int milfs_init (struct vfsconf *vfsp) {
  return 0;
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
