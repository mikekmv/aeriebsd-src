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

#define	milfs_btodb(x)		btodb(MILFS_BBSIZE + (x))
#define	milfs_blkno(mmp, mbp)	milfs_btodb((mbp)->mb_cgno * (mmp)->mm_cgsize \
				    + (mbp)->mb_cgblk * (mmp)->mm_bsize)

extern int (**milfs_vnodeop_p)(void *v);
extern struct pool milfs_block_pool;
extern struct pool milfs_inode_pool;

#endif /* _MILFS_EXTERN_H_ */
