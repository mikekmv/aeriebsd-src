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
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/vnode.h>

#include <milfs/milfs.h>
#include <milfs/milfs_extern.h>

int (**milfs_vnodeop_p)(void *v);

struct vnodeopv_entry_desc milfs_vnodeop_entries[] = {
	{ &vop_default_desc, eopnotsupp },
	{ &vop_lookup_desc, milfs_lookup },		/* lookup */
	{ &vop_create_desc, milfs_create },		/* create */
	{ &vop_mknod_desc, milfs_mknod },		/* mknod */
	{ &vop_open_desc, milfs_open },			/* open */
	{ &vop_close_desc, milfs_close },		/* close */
	{ &vop_access_desc, milfs_access },		/* access */
	{ &vop_getattr_desc, milfs_getattr },		/* getattr */
	{ &vop_setattr_desc, milfs_setattr },		/* setattr */
	{ &vop_read_desc, milfs_read },			/* read */
	{ &vop_write_desc, milfs_write },		/* write */
	{ &vop_ioctl_desc, milfs_ioctl },		/* ioctl */
	{ &vop_poll_desc, milfs_poll },			/* poll */
	{ &vop_kqfilter_desc, milfs_kqfilter },		/* kqfilter */
	{ &vop_revoke_desc, milfs_revoke },		/* revoke */
	{ &vop_fsync_desc, milfs_fsync },		/* fsync */
	{ &vop_remove_desc, milfs_remove },		/* remove */
	{ &vop_link_desc, milfs_link },			/* link */
	{ &vop_rename_desc, milfs_rename },		/* rename */
	{ &vop_mkdir_desc, milfs_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, milfs_rmdir },		/* rmdir */
	{ &vop_symlink_desc, milfs_symlink },		/* symlink */
	{ &vop_readdir_desc, milfs_readdir },		/* readdir */
	{ &vop_abortop_desc, milfs_abortop },		/* abortop */
	{ &vop_inactive_desc, milfs_inactive },		/* inactive */
	{ &vop_reclaim_desc, milfs_reclaim },		/* reclaim */
	{ &vop_lock_desc, milfs_lock },			/* lock */
	{ &vop_unlock_desc, milfs_unlock },		/* unlock */
	{ &vop_bmap_desc, milfs_bmap },			/* bmap */
	{ &vop_strategy_desc, milfs_strategy },		/* strategy */
	{ &vop_print_desc, milfs_print },		/* print */
	{ &vop_islocked_desc, milfs_islocked },		/* islocked */
	{ &vop_pathconf_desc, milfs_pathconf },		/* pathconf */
	{ &vop_advlock_desc, milfs_advlock },		/* advlock */
	{ &vop_reallocblks_desc, milfs_reallocblks },	/* reallocblks */
	{ &vop_bwrite_desc, milfs_bwrite },		/* bwrite */
	{ NULL, NULL }
};

struct vnodeopv_desc milfs_vnodeop_opv_desc = {
	&milfs_vnodeop_p,
	milfs_vnodeop_entries
};

/*
 * This function is semantically overloaded, so it has been chopped off into
 * smaller functions, which are called according to the lookup operation.
 */
int
milfs_lookup(void *v)
{
	int error, nameiop;
	struct componentname *cnp;
	struct milfs_inode *dmip;
	struct vnode *dvp, **vpp;
	struct vop_lookup_args *ap;

	ap = v;
	cnp = ap->a_cnp;
	dvp = ap->a_dvp;
	vpp = ap->a_vpp;
	dmip = (struct milfs_inode *)dvp->v_data;

	/*
	 * Make sure we are operating on a directory, and that it is acessible.
	 */

	if (dvp->v_type != VDIR || (dmip->mi_mode & MILFS_MODE_DIRECTORY) == 0)
		return (ENOTDIR);

	error = VOP_ACCESS(dvp, VEXEC, cnp->cn_cred, cnp->cn_proc);
	if (error)
		return (error);

	/*
	 * Check whether the request has been cached.
	 */

	error = cache_lookup(dvp, vpp, cnp);
	if (error != -1)
		return (error);

	nameiop = cnp->cn_nameiop & OPMASK;
	switch (nameiop) {
	case LOOKUP:
		error = milfs_lookup_only(dvp, vpp, cnp);
		break;
	case CREATE:
		error = milfs_lookup_create(dvp, vpp, cnp);
		break;
	case DELETE:
		error = milfs_lookup_delete(dvp, vpp, cnp);
		break;
	case RENAME:
		error = milfs_lookup_rename(dvp, vpp, cnp);
		break;
	default:
		panic("milfs_lookup: unknown nameiop %d", nameiop);
	}

	if (error == 0) {
		if (cnp->cn_flags & MAKEENTRY)
			cache_enter(dvp, *vpp, cnp);
	}

	return (error);
}

int milfs_create(void *v) {
  printf("milfs_create called!\n");
  return (EINVAL);
}

int milfs_mknod(void *v) {
  printf("milfs_mknod called!\n");
  return (EINVAL);
}

/*
 * Nothing to do.
 */
int
milfs_open(void *v)
{
	return (0);
}

int milfs_close(void *v) {
  printf("milfs_close called!\n");
  return (EINVAL);
}

int
milfs_access(void *v)
{
	mode_t mode;
	struct milfs_inode *mip;
	struct vnode *vp;
	struct vop_access_args *ap;

	ap = v;
	vp = ap->a_vp;
	mip = (struct milfs_inode *)vp->v_data;

	return (vaccess(vp->v_type, mip->mi_mode, mip->mi_xuid, mip->mi_xgid,
	    mode, ap->a_cred));
}

int
milfs_getattr(void *v)
{
	struct milfs_inode *mip;
	struct vnode *vp;
	struct vattr *vap;
	struct vop_getattr_args *ap;

	ap = v;
	vp = ap->a_vp;
	vap = ap->a_vap;
	mip = (struct milfs_inode *)vp->v_data;

	vap->va_fileid = mip->mi_inode;
	vap->va_mode = mip->mi_mode & ~MILFS_MODE_HASSTATIC;
	vap->va_nlink = mip->mi_nlink;
	vap->va_uid = mip->mi_xuid;
	vap->va_gid = mip->mi_xgid;
	vap->va_ctime.tv_sec = mip->mi_changesec;
	vap->va_ctime.tv_nsec = mip->mi_changeusec * 1000;
	vap->va_atime.tv_sec = vap->va_ctime.tv_sec;
	vap->va_atime.tv_nsec = vap->va_ctime.tv_nsec;
	vap->va_mtime.tv_sec = vap->va_ctime.tv_sec;
	vap->va_mtime.tv_nsec = vap->va_ctime.tv_nsec;
	vap->va_size = mip->mi_size;
	vap->va_type = vp->v_type;

	return (0);
}

int milfs_setattr(void *v) {
  printf("milfs_setattr called!\n");
  return (EINVAL);
}

int milfs_read(void *v) {
  printf("milfs_read called!\n");
  return (EINVAL);
}

int milfs_write(void *v) {
  printf("milfs_write called!\n");
  return (EINVAL);
}

int milfs_ioctl(void *v) {
  printf("milfs_ioctl called!\n");
  return (EINVAL);
}

int milfs_poll(void *v) {
  printf("milfs_poll called!\n");
  return (EINVAL);
}

int milfs_kqfilter(void *v) {
  printf("milfs_kqfilter called!\n");
  return (EINVAL);
}

int milfs_revoke(void *v) {
  printf("milfs_revoke called!\n");
  return (EINVAL);
}

int milfs_fsync(void *v) {
  printf("milfs_fsync called!\n");
  return (EINVAL);
}

int milfs_remove(void *v) {
  printf("milfs_remove called!\n");
  return (EINVAL);
}

int milfs_link(void *v) {
  printf("milfs_link called!\n");
  return (EINVAL);
}

int milfs_rename(void *v) {
  printf("milfs_rename called!\n");
  return (EINVAL);
}

int milfs_mkdir(void *v) {
  printf("milfs_mkdir called!\n");
  return (EINVAL);
}

int milfs_rmdir(void *v) {
  printf("milfs_rmdir called!\n");
  return (EINVAL);
}

int milfs_symlink(void *v) {
  printf("milfs_symlink called!\n");
  return (EINVAL);
}

int milfs_readdir(void *v) {
  printf("milfs_readdir called!\n");
  return (EINVAL);
}

int milfs_abortop(void *v) {
  printf("milfs_abortop called!\n");
  return (EINVAL);
}

int milfs_inactive(void *v) {
  printf("milfs_inactive called!\n");
  return (EINVAL);
}

int milfs_reclaim(void *v) {
  printf("milfs_reclaim called!\n");
  return (EINVAL);
}

int
milfs_lock(void *v)
{
	return (0);
}

int
milfs_unlock(void *v)
{
	return (0);
}

int milfs_bmap(void *v) {
  printf("milfs_bmap called!\n");
  return (EINVAL);
}

int milfs_strategy(void *v) {
  printf("milfs_strategy called!\n");
  return (EINVAL);
}

int milfs_print(void *v) {
  printf("milfs_print called!\n");
  return (EINVAL);
}

int milfs_islocked(void *v) {
  printf("milfs_islocked called!\n");
  return (EINVAL);
}

int milfs_pathconf(void *v) {
  printf("milfs_pathconf called!\n");
  return (EINVAL);
}

int milfs_advlock(void *v) {
  printf("milfs_advlock called!\n");
  return (EINVAL);
}

int milfs_reallocblks(void *v) {
  printf("milfs_reallocblks called!\n");
  return (EINVAL);
}

int milfs_bwrite(void *v) {
  printf("milfs_bwrite called!\n");
  return (EINVAL);
}
