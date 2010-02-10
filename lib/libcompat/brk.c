/*
 * Copyright (c) 2010 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

extern char end;
char *__curbrk = &end;

void *
sbrk(int incr)
{
	void *rv;

	rv = __curbrk;
	if (brk(__curbrk + incr) < 0)
		return NULL;
	return rv;
}

int
brk(void *addr)
{
	u_long pgsz = (u_long)getpagesize();
	u_long old, new;

	if ((char *)addr <= &end)
		return 0;

	old = ((u_long)__curbrk + pgsz - 1) & ~(pgsz - 1);
	new = ((u_long)addr + pgsz - 1) & ~(pgsz - 1);
	if (new == old) {
		__curbrk = addr;
		return 0;
	}

	if (new > old) {
		if (mmap((void *)old, new - old, PROT_READ | PROT_WRITE,
		    MAP_ANON | MAP_FIXED | MAP_PRIVATE, -1, 0) == MAP_FAILED) {
			int save_errno = errno;
			fprintf(stderr, "sbrk: grow %lu failed\n", new - old);
			errno = save_errno;
			return -1;
		}
	} else {
		if (munmap((void *)new, old - new) < 0)
			return -1;
	}

	__curbrk = addr;
	return 0;
}
