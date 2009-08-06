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

#ifndef _MILFS_PARAM_H_
#define _MILFS_PARAM_H_

/*
 * File system parameters.
 */
#define	MILFS_CGSIZE_MIN	0x00002000	/* 8KB */
#define	MILFS_CGSIZE_MAX	0x80000000	/* 2GB */
#define	MILFS_CGSIZE_DEFAULT	0x00020000	/* 128KB */
#define	MILFS_BSIZE_MIN		0x00000200	/* 512B */
#define	MILFS_BSIZE_MAX		0x00040000	/* 256KB */
#define	MILFS_BSIZE_DEFAULT	0x00000800	/* 2KB */
#define	MILFS_BBSIZE		0x00002000	/* 8KB */

/*
 * Reserved inode numbers.
 */
#define	MILFS_INODE_DIRECTORY	0
#define	MILFS_INODE_ROOT	1

/*
 * Inode modes.
 */
#define	MILFS_MODE_DIRECTORY	0x0001

/*
 * Magic for dynamic inodes.
 */
#define	MILFS_DINODE_MAGIC	0xfbd9a736

/*
 * Magic for cylinder group descriptors.
 */
#define	MILFS_CGDESC_MAGIC	0x15ec0a92

#endif /* _MILFS_PARAM_H_ */
