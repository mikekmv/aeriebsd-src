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
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/mount.h>

#include <milfs/milfs.h>
#include <milfs/milfs_extern.h>

/*
 * Namei 'lookup' operation.
 */
int
milfs_lookup_only(struct vnode *dvp, struct vnode **vpp,
    struct componentname *cnp)
{
	int error, i, nentries;
	struct buf *bp;
	struct milfs_block *dmbp;
	struct milfs_dirent *dep;
	struct milfs_inode *dmip;
	struct milfs_mount *mmp;
	struct mount *mp;

	if (cnp->cn_namelen > MILFS_NAMESIZE)
		return (EINVAL);

	/*
	 * '.' always exists.
	 */

	if (cnp->cn_namelen == 1 && cnp->cn_nameptr[0] == '.') {
		vref(dvp);
		*vpp = dvp;
		return (0);
	}

	/*
	 * do a search through the directory blocks. frequently accessed
	 * entries should hang around, either in the buffer cache or in
	 * privileged positions in the splay tree.
	 */

	dmip = (struct milfs_inode *)dvp->v_data;
	mp = dvp->v_mount;
	mmp = (struct milfs_mount *)mp->mnt_data;

	SPLAY_FOREACH(dmbp, milfs_block_tree, &dmip->mi_blktree) {
		error = bread(mmp->mm_devvp, milfs_blkno(mmp, dmbp),
		    mmp->mm_bsize, cnp->cn_cred, &bp);
		if (error) {
			brelse(bp);
			return (error);
		}
		nentries = dmbp->mb_blksize / sizeof(struct milfs_dirent);
		dep = (struct milfs_dirent *)bp->b_data;
		for (i = 0; i < nentries; i++) {
			if (strncmp(dep[i].de_entry, cnp->cn_nameptr,
			    cnp->cn_namelen) == 0) {	/* found the entry. */
				error = VFS_VGET(mp, dep[i].de_inode, vpp);
				brelse(bp);
				return (error);
			}
		}
		brelse(bp);
	}

	return (ENOENT);
}

/*
 * Namei 'create' operation.
 */
int
milfs_lookup_create(struct vnode *dvp, struct vnode **vpp,
    struct componentname *cnp)
{
	*vpp = NULL;
	return (ENOENT);
}

/*
 * Namei 'delete' operation.
 */
int
milfs_lookup_delete(struct vnode *dvp, struct vnode **vpp,
    struct componentname *cnp)
{
	*vpp = NULL;
	return (ENOENT);
}

/*
 * Namei 'rename' operation.
 */
int
milfs_lookup_rename(struct vnode *dvp, struct vnode **vpp,
    struct componentname *cnp)
{
	*vpp = NULL;
	return (ENOENT);
}
