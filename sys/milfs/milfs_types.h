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
	u_int32_t	di_magic;	/* Inode magic. */
};

/*
 * On-disk layout of a cylinder group descriptor.
 */
struct milfs_cgdesc {
	u_int32_t	cd_bsize;	/* Cylinder group block size. */
	u_int32_t	cd_spare;	/* Reserved; currently unused. */
	u_int32_t	cd_cgsize;	/* Cylinder group size. */
	u_int32_t	cd_magic;	/* Cylinder group magic. */
};

/*
 * In-memory representation of a mounted MILFS file system.
 */
struct milfs_mount {
	u_int32_t	mm_bsize;	/* File system block size. */
	u_int32_t	mm_cgsize;	/* File system cylinder group size. */
	struct vnode   *mm_devvp;	/* File system device vnode pointer. */
	u_int32_t	mm_ncg;		/* Number of cylinder groups. */
	u_int32_t	mm_blkpercg;	/* Blocks per cylinder group. */
};

#endif /* _MILFS_TYPES_H_ */
