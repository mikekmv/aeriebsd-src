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
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/kthread.h>

#include <milfs/milfs.h>
#include <milfs/milfs_extern.h>

SPLAY_GENERATE(milfs_block_tree, milfs_block, mb_nodes, milfs_block_cmp);
SPLAY_GENERATE(milfs_inode_tree, milfs_inode, mi_nodes, milfs_inode_cmp);

/*
 * Local function for sorting the block tree of a mount point.
 */
int
milfs_block_cmp(struct milfs_block *mbp1, struct milfs_block *mbp2)
{
	if (mbp1->mb_offset < mbp2->mb_offset) {
		if (mbp1->mb_blksize > mbp2->mb_blksize)
			return (0);	/* 1 includes 2 */
		else
			return (-1);
	} else if (mbp1->mb_offset > mbp2->mb_offset) {
		if (mbp2->mb_blksize > mbp1->mb_blksize)
			return (0);	/* 2 includes 1 */
		else
			return (1);
	}
	return (0);
}

/*
 * Local function for sorting the inode tree of a mount point.
 */
int
milfs_inode_cmp(struct milfs_inode *mip1, struct milfs_inode *mip2)
{
	if (mip1->mi_inode > mip2->mi_inode)
		return (1);
	else if (mip1->mi_inode < mip2->mi_inode)
		return (-1);
	return (0);
}

/*
 * Look up an inode in the inode tree of a mount point.
 */
struct milfs_inode *
milfs_inolookup(struct milfs_mount *mmp, u_int64_t ino)
{
	struct milfs_inode mi;

	mi.mi_inode = ino;

	return (SPLAY_FIND(milfs_inode_tree, &mmp->mm_inotree, &mi));
}

/*
 * Allocate a new inode structure. Insert it in the inode tree.
 */
struct milfs_inode *
milfs_inocreate(struct milfs_mount *mmp, u_int64_t ino)
{
	struct milfs_inode *mip;

	mip = pool_get(&milfs_inode_pool, PR_WAITOK | PR_ZERO);
	mip->mi_inode = ino;
	SPLAY_INIT(&mip->mi_blktree);

	SPLAY_INSERT(milfs_inode_tree, &mmp->mm_inotree, mip);

	return (mip);
}

/*
 * Look up an inode in the inode directory.
 */
struct milfs_inode *
milfs_inodirlookup(struct milfs_mount *mmp, struct milfs_inode *mip,
    u_int64_t ino)
{
	daddr64_t blkno;
	int error, i, nentries;
	struct buf *bp;
	struct milfs_block *dmbp;
	struct milfs_inode *dmip;
	struct milfs_sinode *sip;
	struct proc *p;

	dmip = milfs_inolookup(mmp, MILFS_INODE_DIRECTORY);
	if (dmip == NULL)
		panic("milfs_inodirlookup: no inode directory");

	p = curproc;

	SPLAY_FOREACH(dmbp, milfs_block_tree, &dmip->mi_blktree) {
		/* XXX pedro: ugly. improve. */
		blkno = milfs_btodb(dmbp->mb_cgno * mmp->mm_cgsize +
		    dmbp->mb_cgblk * mmp->mm_bsize);
		error = bread(mmp->mm_devvp, blkno, mmp->mm_bsize, p->p_ucred,
		    &bp);
		if (error) {
			brelse(bp);
			printf("milfs_inodirlookup: read error %d\n", error);
			return (NULL);
		}

		nentries = dmbp->mb_blksize / sizeof(struct milfs_sinode);
		sip = (struct milfs_sinode *)bp->b_data;

		for (i = 0; i < nentries; i++) {
			if (sip[i].si_inode == ino) {
				/*
				 * XXX pedro: we should check if another
				 * process was able to get the inode while we
				 * slept.
				 */
				if (mip == NULL) {
					mip = milfs_inocreate(mmp, ino);
					if (mip == NULL) {
						brelse(bp);
						return (NULL);
					}
				}
				mip->mi_birthsec = sip[i].si_birthsec;
				mip->mi_birthnsec = sip[i].si_birthnsec;
				mip->mi_changesec = sip[i].si_changesec;
				mip->mi_changeusec = sip[i].si_changeusec;
				mip->mi_gen = sip[i].si_gen;
				mip->mi_xuid = sip[i].si_xuid;
				mip->mi_xgid = sip[i].si_xgid;
				mip->mi_mode = sip[i].si_mode |
				    MILFS_MODE_HASSTATIC;
				mip->mi_nlink = sip[i].si_nlink;

				brelse(bp);

				return (mip);
			}
		}

		brelse(bp);
	}

	return (NULL);
}

/*
 * Look up a block in the block tree of an inode.
 */
struct milfs_block *
milfs_blklookup(struct milfs_inode *mip, u_int32_t offset, u_int32_t size)
{
	struct milfs_block mb;

	mb.mb_offset = offset;
	mb.mb_blksize = size;

	return (SPLAY_FIND(milfs_block_tree, &mip->mi_blktree, &mb));
}

/*
 * Allocate a new block structure. Insert it in the block tree of the inode.
 */
struct milfs_block *
milfs_blkcreate(struct milfs_inode *mip, u_int32_t offset, u_int32_t size)
{
	struct milfs_block *mbp;

	mbp = pool_get(&milfs_block_pool, PR_WAITOK | PR_ZERO);
	mbp->mb_inode = mip->mi_inode;
	mbp->mb_offset = offset;
	mbp->mb_blksize = size;

	SPLAY_INSERT(milfs_block_tree, &mip->mi_blktree, mbp);

	mip->mi_size += size;

	return (mbp);
}

/*
 * Remove a block structure from the block tree of an inode.
 */
void
milfs_blkdelete(struct milfs_inode *mip, struct milfs_block *mbp)
{
	SPLAY_REMOVE(milfs_block_tree, &mip->mi_blktree, mbp);
	mip->mi_size -= mbp->mb_blksize;
	pool_put(&milfs_block_pool, mbp);
}

/*
 * See whether 'dip' is a more recent version of 'mbp'.
 */
int
milfs_blkisnew(struct milfs_dinode *dip, struct milfs_block *mbp)
{
	if (dip->di_gen > mbp->mb_gen)
		return (1);	/* Yes. */

	if (dip->di_gen < mbp->mb_gen)
		return (0);	/* No. */

	if (dip->di_modsec > mbp->mb_modsec)
		return (1);

	if (dip->di_modsec < mbp->mb_modsec)
		return (0);

	if (dip->di_modusec > mbp->mb_modusec)
		return (1);

	if (dip->di_modusec < mbp->mb_modusec)
		return (0);

	panic("milfs_blkisnew: identical block attributes");
}

/*
 * Scan an inode block.
 */
int
milfs_scanblk(struct milfs_mount *mmp, struct milfs_inode *mip,
  struct milfs_dinode *dip, int cg, int blk)
{
	struct milfs_block *mbp;

	/*
	 * Do we already have a block describing this part of the inode?
	 */
	mbp = milfs_blklookup(mip, dip->di_offset, dip->di_size);
	if (mbp != NULL) {
		/*
		 * If so, see whether 'dip' is a more preferable (recent)
		 * version of 'mbp'.
		 */
		if (milfs_blkisnew(dip, mbp) == 0) {
			/*
			 * If it is not, keep 'mbp' and return.
			 */
			return (0);
		}
		/*
		 * Otherwise, remove 'mbp' from the tree and replace it by
		 * 'dip'.
		 */
		clrbit(mmp->mm_cg[cg].cg_bmap, blk);
		milfs_blkdelete(mip, mbp);
	}

	mbp = milfs_blkcreate(mip, dip->di_offset, dip->di_size);
	if (mbp == NULL)
		return (ENOMEM);

	mbp->mb_cgno = cg;
	mbp->mb_cgblk = blk;
	mbp->mb_modsec = dip->di_modsec;
	mbp->mb_modusec = dip->di_modusec;
	mbp->mb_gen = dip->di_gen;

	setbit(mmp->mm_cg[cg].cg_bmap, blk);

	return (0);
}

/*
 * Scan for inode blocks.
 */
int
milfs_scanino(struct milfs_mount *mmp, struct milfs_dinode *dip, int cg)
{
	int error, i;
	struct milfs_inode *mip;
	struct milfs_cgdesc *cdp;

	/*
	 * Skip unallocated cylinder groups.
	 */
	cdp = (struct milfs_cgdesc *)&dip[mmp->mm_blkpercg - 1];
	if (cdp->cd_magic != MILFS_CGDESC_MAGIC)
		return (0);

	for (i = 0; i < mmp->mm_blkpercg - 1; i++) {
		/*
		 * Skip unallocated entries.
		 */
		if (dip[i].di_size == 0)
			continue;
		/*
		 * Is it the first time we are seeing this inode?
		 */
		mip = milfs_inolookup(mmp, dip[i].di_inode);
		if (mip == NULL) {
			mip = milfs_inocreate(mmp, dip[i].di_inode);
			if (mip == NULL)
				return (ENOMEM);
		}
		error = milfs_scanblk(mmp, mip, &dip[i], cg, i);
		if (error)
			return (error);
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
	u_int64_t cgblk, psize;

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

	if (i >= 32)
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

	psize = DL_GETPSIZE(pinfo.part) * pinfo.disklab->d_secsize;
	ncg = (psize - MILFS_BBSIZE) / cgsize;
	if (ncg == 0)
		return (EINVAL);

	mmp = malloc(sizeof(struct milfs_mount), M_MILFS, M_WAITOK | M_ZERO);
	mmp->mm_bsize = bsize;
	mmp->mm_cgsize = cgsize;
	mmp->mm_devvp = vp;
	mmp->mm_ncg = ncg;
	mmp->mm_blkpercg = mmp->mm_cgsize / mmp->mm_bsize;
	mmp->mm_cg = malloc(sizeof(struct milfs_cg) * mmp->mm_ncg, M_MILFS,
	    M_WAITOK | M_ZERO);

	for (i = 0; i < mmp->mm_ncg; i++)
		mmp->mm_cg[i].cg_bmap = malloc((mmp->mm_blkpercg / NBBY) *
		    sizeof(unsigned char), M_MILFS, M_WAITOK | M_ZERO);

	SPLAY_INIT(&mmp->mm_inotree);

	/*
	 * Reconstruct the inode and block trees.
	 */
	error = milfs_scancg(mmp, milfs_scanino);
	if (error) {
		free(mmp, M_MILFS);
		return (error);
	}

	mp->mnt_data = (void *)mmp;

	if ((error = kthread_create(milfs_cleaner, mmp, &mmp->mm_cleaner,
	    "milc", NULL))) {
		free(mmp, M_MILFS);
		/* XXX cleanup inodes */
		return (error);
	}

	return (0);
}

int
milfs_getfreeblk(struct milfs_mount *mmp, int cg)
{
	int i, j, nbytes, nzeros;

	nbytes = mmp->mm_blkpercg / NBBY;
	for (i = 0; i < nbytes; i++) {
		nzeros = mmp->mm_cg[cg].cg_bmap[i] ^ 0xff;
		if (nzeros == 0)
			continue;
		for (j = 0; j < NBBY; j++) {
			if ((nzeros & 0x1) == 1)
				return (i * NBBY + j);
			nzeros = nzeros >> 1;
		}
	}

	return (-1);
}
