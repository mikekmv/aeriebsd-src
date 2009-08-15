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
	int error, i, newblk;
	struct milfs_inode *dmip;
	struct milfs_mount *mmp;

	/*
	 * If not at the last component name, do a regular lookup.
	 */
	if ((cnp->cn_flags & ISLASTCN) == 0)
		return (milfs_lookup_only(dvp, vpp, cnp));

	/*
	 * Last component name. Do an initial lookup to see whether the entry
	 * already exists.
	 */
	 error = milfs_lookup_only(dvp, vpp, cnp);
	 if (error && error != ENOENT) {
		 *vpp = NULL;
		 return (error);
	 }

	/*
	 * from ufs_lookup(): "Access for write is interpreted as allowing
	 * creation of files in the directory."
	 */
	error = VOP_ACCESS(dvp, VWRITE, cnp->cn_cred, cnp->cn_proc);
	if (error) {
		*vpp = NULL;
		return (error);
	}

	if (dmip->mi_curblk == NULL)
		dmip->mi_curblk = SPLAY_MAX(milfs_block_tree,
		    &dmip->mi_blktree);

	/*
	 * See if there is room for the new entry.
	 */
	mmp = (struct milfs_mount *)dmip->mi_vnode->v_mount->mnt_data;
	if (dmip->mi_curblk->mb_blksize + sizeof(struct milfs_dirent) >
	    mmp->mm_bsize) {
		/*
		 * If there is not, look at the bitmaps for a new block.
		 * Mark it as busy and allocate a structure for it.
		 */
		newblk = -1;
		for (i = dmip->mi_curblk->mb_cgno; i < mmp->mm_ncg; i++) {
			newblk = milfs_getfreeblk(mmp, i);
			if (newblk != -1)
				break;
		}
		if (newblk == -1) {
			*vpp = NULL;
			return (ENOSPC);
		}
		dmip->mi_curblk = milfs_blkcreate(dmip,
		   dmip->mi_curblk->mb_offset + mmp->mm_bsize,
		   sizeof(struct milfs_dirent));
	}

	cnp->cn_flags |= SAVENAME;
	if (cnp->cn_flags & LOCKPARENT) {
		VOP_UNLOCK(dvp, 0, cnp->cn_proc);
		cnp->cn_flags |= PDIRUNLOCK;
	}

	return (EJUSTRETURN);
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
