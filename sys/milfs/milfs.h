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

#ifndef _MILFS_TYPES_H_
#define _MILFS_TYPES_H_

/*
 * File system parameters.
 */
#define	MILFS_CGSIZE_MIN	0x00002000	/* 8KB */
#define	MILFS_CGSIZE_MAX	0x80000000	/* 2GB */
#define	MILFS_CGSIZE_DEFAULT	0x00020000	/* 128KB */
#define	MILFS_BSIZE_MIN		0x00000200	/* 512B */
#define	MILFS_BSIZE_MAX		0x00080000	/* 512KB */
#define	MILFS_BSIZE_DEFAULT	0x00000800	/* 2KB */
#define	MILFS_BBSIZE		0x00002000	/* 8KB */
#define	MILFS_NAMESIZE		504

/*
 * Reserved inode numbers.
 */
#define	MILFS_INODE_DIRECTORY	0
#define	MILFS_INODE_ROOT	1

/*
 * Inode modes.
 */
#define	MILFS_MODE_DIRECTORY	0x0001
#define	MILFS_MODE_HASSTATIC	0x8000	/* Never committed to disk. */

/*
 * Magic for cylinder group descriptors.
 */
#define	MILFS_CGDESC_MAGIC	0x15ec0a92

/*
 * On-disk layout of a static MILFS inode.
 */
struct milfs_sinode {
	u_int64_t	si_inode;	/* Inode number. */
	int32_t		si_spare0[3];	/* Reserved; currently unused. */
	int64_t		si_birthsec;	/* Inode creation second. */
	int32_t		si_birthnsec;	/* Inode creation nanosecond. */
	int32_t		si_changesec;	/* Last inode change second. */
	int32_t		si_changeusec;	/* Last inode change microsecond. */
	u_int32_t	si_gen;		/* Generation number. */
	u_int32_t	si_xuid;	/* File owner. */
	u_int32_t	si_xgid;	/* File group. */
	int16_t		si_spare1[4];	/* Reserved; currently unused. */
	int16_t		si_mode;	/* File mode. */
	int16_t		si_nlink;	/* File link count. */
};

/*
 * On-disk layout of a dynamic MILFS inode.
 */
struct milfs_dinode {
	u_int64_t	di_inode;	/* Inode number. */
	u_int64_t	di_offset;	/* Block offset. */
	int32_t		di_modsec;	/* Modification second. */
	int32_t		di_modusec;	/* Modification microsecond. */
	u_int32_t	di_gen;		/* Dynamic inode generation. */
	u_int32_t	di_size;	/* Dynamic inode size. */
};

/*
 * On-disk layout of a cylinder group descriptor.
 */
struct milfs_cgdesc {
	u_int32_t	cd_bsize;	/* Cylinder group block size. */
	u_int32_t	cd_spare0;	/* Reserved; currently unused. */
	u_int32_t	cd_cgsize;	/* Cylinder group size. */
	u_int32_t	cd_magic;	/* Cylinder group magic. */
	u_int32_t	cd_spare1[4];
};

/*
 * On-disk layout of a directory entry.
 */
struct milfs_dirent {
	u_int64_t       de_inode;			/* Entry inode. */
	unsigned char   de_entry[MILFS_NAMESIZE];	/* Entry name. */
};

/*
 * In-memory representation of a MILFS inode block.
 */
struct milfs_block {
	SPLAY_ENTRY(milfs_block) mb_nodes;
	u_int64_t	mb_inode;	/* Inode number. */
	u_int32_t	mb_offset;	/* Block offset. */
	u_int32_t	mb_blksize;	/* Block size. */
	u_int32_t	mb_cgno;	/* Cylinder group. */
	u_int32_t	mb_cgblk;	/* Offset in group. */
	int32_t		mb_modsec;	/* Modif. second. */
	int32_t		mb_modusec;	/* Modif. usecond. */
	u_int32_t	mb_gen;		/* Block generation. */
};

SPLAY_HEAD(milfs_block_tree, milfs_block);

/*
 * In-memory representation of a MILFS inode.
 */
struct milfs_inode {
	SPLAY_ENTRY(milfs_inode) mi_nodes;
	struct milfs_block *mi_curblk;	/* Inode current block. */
	u_int64_t	mi_inode;       /* Inode number. */
	u_int64_t	mi_size;        /* Inode size. */
	struct milfs_block_tree mi_blktree; /* Inode blocks. */
	int64_t		mi_birthsec;
	int32_t		mi_birthnsec;
	int32_t		mi_changesec;
	int32_t		mi_changeusec;
	u_int32_t	mi_gen;
	u_int32_t	mi_xuid;
	u_int32_t	mi_xgid;
	int16_t		mi_mode;
	int16_t		mi_nlink;
	struct vnode	*mi_vnode;	/* Backpointer to v. */
};

SPLAY_HEAD(milfs_inode_tree, milfs_inode);

/*
 * In-memory representation of a MILFS cylinder group.
 */
struct milfs_cg {
	unsigned char  *cg_bmap;	/* Block bitmap. */
};

/*
 * In-memory representation of a mounted MILFS file system.
 */
struct milfs_mount {
	struct milfs_inode_tree mm_inotree;
	struct milfs_cg *mm_cg;		/* Array of cylinder groups. */
	struct vnode   *mm_devvp;	/* File system device vnode pointer. */
	struct proc    *mm_cleaner;	/* Cleaner thread to kill */
	u_int32_t	mm_flags;	/* Flags */
#define	MILFS_GONE	0x00000001	/* being unmounted */
	u_int32_t	mm_bsize;	/* File system block size. */
	u_int32_t	mm_cgsize;	/* File system cylinder group size. */
	u_int32_t	mm_ncg;		/* Number of cylinder groups. */
	u_int32_t	mm_blkpercg;	/* Blocks per cylinder group. */
};

#endif /* _MILFS_TYPES_H_ */
