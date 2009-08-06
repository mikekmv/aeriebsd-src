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

#ifndef _MILFS_PROTO_H_
#define _MILFS_PROTO_H_

/* milfs_vfsops.c */
int	milfs_mount(struct mount *, const char *, void *, struct nameidata *,
	    struct proc *);
int	milfs_start(struct mount *, int, struct proc *);
int	milfs_unmount(struct mount *, int, struct proc *);
int	milfs_root(struct mount *, struct vnode **);
int	milfs_quotactl(struct mount *, int, uid_t, caddr_t, struct proc *);
int	milfs_statfs(struct mount *, struct statfs *, struct proc *);
int	milfs_sync(struct mount *, int, struct ucred *, struct proc *);
int	milfs_vget(struct mount *, ino_t, struct vnode **);
int	milfs_fhtovp(struct mount *, struct fid *, struct vnode **);
int	milfs_vptofh(struct vnode *, struct fid *);
int	milfs_init(struct vfsconf *);
int	milfs_sysctl(int *, u_int, void *, size_t *, void *, size_t,
	    struct proc *);
int	milfs_checkexp(struct mount *, struct mbuf *, int *, struct ucred **);

/* milfs_vnops.c */
int	milfs_lookup(void *);
int	milfs_create(void *);
int	milfs_mknod(void *);
int	milfs_open(void *);
int	milfs_close(void *);
int	milfs_access(void *);
int	milfs_getattr(void *);
int	milfs_setattr(void *);
int	milfs_read(void *);
int	milfs_write(void *);
int	milfs_ioctl(void *);
int	milfs_poll(void *);
int	milfs_kqfilter(void *);
int	milfs_revoke(void *);
int	milfs_fsync(void *);
int	milfs_remove(void *);
int	milfs_link(void *);
int	milfs_rename(void *);
int	milfs_mkdir(void *);
int	milfs_rmdir(void *);
int	milfs_symlink(void *);
int	milfs_readdir(void *);
int	milfs_readlink(void *);
int	milfs_abortop(void *);
int	milfs_inactive(void *);
int	milfs_reclaim(void *);
int	milfs_lock(void *);
int	milfs_unlock(void *);
int	milfs_bmap(void *);
int	milfs_strategy(void *);
int	milfs_print(void *);
int	milfs_islocked(void *);
int	milfs_pathconf(void *);
int	milfs_advlock(void *);
int	milfs_reallocblks(void *);
int	milfs_bwrite(void *);

/* milfs_subr.c */
int	milfs_mountfs(struct mount *, struct vnode *);
int	milfs_scanblk(struct milfs_mount *, struct milfs_dinode *, int);
int	milfs_scancg(struct milfs_mount *, int (struct milfs_mount *,
	    struct milfs_dinode *, int));
int	milfs_scanid(struct milfs_mount *, struct milfs_dinode *, int);
int	milfs_scanino(struct milfs_mount *, struct milfs_dinode *, int);

#endif /* _MILFS_PROTO_H_ */
