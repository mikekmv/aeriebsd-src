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
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/mount.h>

#include <milfs/milfs_param.h>
#include <milfs/milfs_types.h>
#include <milfs/milfs_proto.h>
#include <milfs/milfs_extern.h>

/*
 * Scan an inode block.
 */
int
milfs_scanblk(struct milfs_mount *mmp, struct milfs_dinode *dip, int cg)
{
#if 0
	struct milfs_minode *mip;
	struct milfs_mblock *mbp;

	/*
	 * Only scan the block if the corresponding inode is allocated.
	 */
	if (milfs_inochk_alloc(mmp, dip->di_inode) == 0)
		return (0);

	/*
	 * Only scan the block if the corresponding inode is older.
	 */
	if (milfs_inochk_age(mmp, dip->di_modsec, dip->di_modusec) == 0)
		return (0);

	/*
	 * Is it the first time we see this inode?
	 */
	mip = milfs_inolookup(mmp, dip->di_inode);
	if (mip == NULL) {
		mip = milfs_inonew(mmp, dip->di_inode);
		if (mip == NULL)
			return (ENOMEM);
	}

	/*
	 * Is it the first time we see this block?
	 */
	mbp = milfs_blklookup(mmp, dip->di_inode, dip->di_offset);
	if (mbp != NULL) {
		/*
		 * The block has been seen before. Figure out whether this
		 * is a preferable (more recent) copy.
		 */
		if (milfs_blkpref(mmp, mbp, dip) == 0)
			return (0);
		milfs_blkdel(mmp, mbp);
	}

	/*
	 * Insert new block or replace old block.
	 */
	mbp = milfs_blknew(mmp, dip->di_inode, dip->di_offset);
	if (mbp == NULL)
		return (ENOMEM);
#endif
	return (0);
}

/*
 * Scan for blocks belonging to the inode directory.
 */
int
milfs_scanid(struct milfs_mount *mmp, struct milfs_dinode *dip, int cg)
{
	int error, i;

	for (i = 0; i < mmp->mm_blkpercg; i++) {
		/*
		 * Skip unallocated entries.
		 */
		if (dip[i].di_magic != MILFS_DINODE_MAGIC)
			continue;
		if (dip[i].di_inode == MILFS_INODE_DIRECTORY) {
			error = milfs_scanblk(mmp, &dip[i], cg);
			if (error)
				return (error);
		}
	}

	return (0);
}

int
milfs_scancg(struct milfs_mount *mmp, int f(struct milfs_mount *,
    struct milfs_dinode *, int))
{
	int error, i;
	struct buf *bp;
	struct proc *p;
	u_int64_t cgblk;

	p = curproc;

	for (i = 0; i < mmp->mm_ncg; i++) {
		cgblk = milfs_btodb((i + 1) * mmp->mm_cgsize - mmp->mm_bsize);
		error = bread(mmp->mm_devvp, cgblk, mmp->mm_bsize, p->p_ucred,
		    &bp);
		if (error) {
			brelse(bp);
			return (error);
		}
		error = f(mmp, (struct milfs_dinode *)bp->b_data, i);
		brelse(bp);
		if (error)
			return (error);
	}

	return (0);
}

int
milfs_mountfs(struct mount *mp, struct vnode *vp)
{
	int error, i;
	struct buf *bp;
	struct partinfo pinfo;
	struct proc *p;
	struct milfs_cgdesc *cdp;
	struct milfs_mount *mmp;
	u_int32_t bsize, cdoffset, cgsize, ncg;
	u_int64_t cgblk;

	p = curproc;

	/*
	 * Get partition parameters.
	 */

	error = VOP_IOCTL(vp, DIOCGPART, &pinfo, FREAD, p->p_ucred, p);
	if (error)
		return (error);

	if (pinfo.part->p_fstype != FS_MILFS)
		return (EINVAL);

	/*
	 * Find out the file system's cylinder group size. It can range from
	 * MILFS_CGSIZE_MIN (or 2^13) bytes to MILFS_CGSIZE_MAX (or 2^31)
	 * bytes, always at odd powers of 2.
	 */

	cdoffset = DEV_BSIZE - sizeof(struct milfs_dinode);

	for (i = 13; i < 32; i += 2) {
		cgsize = 1 << i;
		cgblk = milfs_btodb(cgsize) - 1;
		if (bread(vp, cgblk, DEV_BSIZE, p->p_ucred, &bp)) {
			brelse(bp);
			continue;
		}

		cdp = (struct milfs_cgdesc *)((char *)bp->b_data + cdoffset);
		if (cdp->cd_magic == MILFS_CGDESC_MAGIC) {
			brelse(bp);
			break;
		}

		brelse(bp);
	}

	if (i > 32)
		return (EINVAL);

	/*
	 * Calculate the corresponding file system block size. Verify that it
	 * is a multiple of the device's sector size.
	 */

	bsize = 1 << ((5 + i) / 2);
	if (bsize < pinfo.disklab->d_secsize ||
	    bsize % pinfo.disklab->d_secsize)
		return (EINVAL);

	/*
	 * Find out the number of cylinder groups in the file system.
	 */

	ncg = ((DL_GETPSIZE(pinfo.part) * pinfo.disklab->d_secsize) -
	    MILFS_BBSIZE) / cgsize;
	if (ncg == 0)
		return (EINVAL);

	mmp = malloc(sizeof(struct milfs_mount), M_MILFS, M_WAITOK | M_ZERO);
	mmp->mm_bsize = bsize;
	mmp->mm_cgsize = cgsize;
	mmp->mm_devvp = vp;
	mmp->mm_ncg = ncg;
	mmp->mm_blkpercg = mmp->mm_cgsize / mmp->mm_bsize;

	/*
	 * Reconstruct the inode directory.
	 */
	error = milfs_scancg(mmp, milfs_scanid);
	if (error) {
		free(mmp, M_MILFS);
		return (error);
	}

	/*
	 * Traverse the inode directory to see which inodes are allocated.
	 */
#if 0
	error = milfs_checkid(mmp);
#endif

	mp->mnt_data = (void *)mmp;

	return (EINVAL);
}
