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

#ifndef _MILFS_EXTERN_H_
#define _MILFS_EXTERN_H_

#include <sys/pool.h>

#define	milfs_btodb(x)	btodb(MILFS_BBSIZE + (x))
#define	VFSTOMILFS(mp)	((struct milfs_mount *)((mp)->mnt_data))

#define	milfs_btodb(x)		btodb(MILFS_BBSIZE + (x))
#define	milfs_blkno(mmp, mbp) \
    milfs_btodb((mbp)->mb_cgno * (mmp)->mm_cgsize + \
    (mbp)->mb_cgblk * (mmp)->mm_bsize)

extern int (**milfs_vnodeop_p)(void *v);
extern struct pool milfs_block_pool;
extern struct pool milfs_inode_pool;

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

/* milfs_cleaner.c */
void	milfs_cleaner(void *);

/* milfs_subr.c */
int	milfs_block_cmp(struct milfs_block *, struct milfs_block *);
int	milfs_inode_cmp(struct milfs_inode *, struct milfs_inode *);
int	milfs_blkisnew(struct milfs_dinode *, struct milfs_block *);
int	milfs_getfreeblk(struct milfs_mount *, int);
int	milfs_mountfs(struct mount *, struct vnode *);
int	milfs_scanblk(struct milfs_mount *, struct milfs_inode *,
	    struct milfs_dinode *, int, int);
int	milfs_scancg(struct milfs_mount *, int (struct milfs_mount *,
	    struct milfs_dinode *, int));
int	milfs_scanid(struct milfs_mount *, struct milfs_dinode *, int);
int	milfs_scanino(struct milfs_mount *, struct milfs_dinode *, int);

struct milfs_block	*milfs_blkcreate(struct milfs_inode *, u_int32_t,
			    u_int32_t);
struct milfs_block	*milfs_blklookup(struct milfs_inode *, u_int32_t,
			    u_int32_t);
struct milfs_inode	*milfs_inocreate(struct milfs_mount *, u_int64_t);
struct milfs_inode	*milfs_inolookup(struct milfs_mount *, u_int64_t);
struct milfs_inode	*milfs_inodirlookup(struct milfs_mount *,
			    struct milfs_inode *, u_int64_t);

void	milfs_blkdelete(struct milfs_inode *, struct milfs_block *);

SPLAY_PROTOTYPE(milfs_block_tree, milfs_block, mb_nodes, milfs_block_cmp);
SPLAY_PROTOTYPE(milfs_inode_tree, milfs_inode, mi_nodes, milfs_inode_cmp);

/* milfs_lookup.c */
int	milfs_lookup_only(struct vnode *, struct vnode **,
	    struct componentname *);
int	milfs_lookup_create(struct vnode *, struct vnode **,
	    struct componentname *);
int	milfs_lookup_rename(struct vnode *, struct vnode **,
	    struct componentname *);
int	milfs_lookup_delete(struct vnode *, struct vnode **,
	    struct componentname *);

#endif /* _MILFS_EXTERN_H_ */
