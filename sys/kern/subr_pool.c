/*-
 * Copyright (c) 1997, 1999, 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Paul Kranenburg; by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/pool.h>
#include <sys/syslog.h>
#include <sys/sysctl.h>

#include <uvm/uvm.h>


/*
 * Pool resource management utility.
 *
 * Memory is allocated in pages which are split into pieces according to
 * the pool item size. Each page is kept on one of three lists in the
 * pool structure: `pr_emptypages', `pr_fullpages' and `pr_partpages',
 * for empty, full and partially-full pages respectively. The individual
 * pool items are on a linked list headed by `ph_itemlist' in each page
 * header. The memory for building the page list is either taken from
 * the allocated pages themselves (for small pool items) or taken from
 * an internal pool of page headers (`phpool').
 */

/* List of all pools */
TAILQ_HEAD(,pool) pool_head = TAILQ_HEAD_INITIALIZER(pool_head);

/* Private pool for page header structures */
struct pool phpool;

struct pool_item_header {
	/* Page headers */
	LIST_ENTRY(pool_item_header)
				ph_pagelist;	/* pool page list */
	TAILQ_HEAD(,pool_item)	ph_itemlist;	/* chunk list for this page */
	SPLAY_ENTRY(pool_item_header)
				ph_node;	/* Off-page page headers */
	int			ph_nmissing;	/* # of chunks in use */
	caddr_t			ph_page;	/* this page's address */
};

struct pool_item {
#ifdef DIAGNOSTIC
	int pi_magic;
#endif
#ifdef DEADBEEF1
#define	PI_MAGIC DEADBEEF1
#else
#define	PI_MAGIC 0xdeafbeef
#endif
	/* Other entries use only this list entry */
	TAILQ_ENTRY(pool_item)	pi_list;
};

#define	POOL_NEEDS_CATCHUP(pp)						\
	((pp)->pr_nitems < (pp)->pr_minitems)

/*
 * Every pool gets a unique serial number assigned to it. If this counter
 * wraps, we're screwed, but we shouldn't create so many pools anyway.
 */
unsigned int pool_serial;

int	 pool_catchup(struct pool *);
void	 pool_prime_page(struct pool *, caddr_t, struct pool_item_header *);
void	 pool_update_curpage(struct pool *);
void	*pool_do_get(struct pool *, int);
void	 pool_do_put(struct pool *, void *);
void	 pr_rmpage(struct pool *, struct pool_item_header *,
	    struct pool_pagelist *);
int	pool_chk_page(struct pool *, const char *, struct pool_item_header *);
struct pool_item_header *pool_alloc_item_header(struct pool *, caddr_t , int);

void	*pool_allocator_alloc(struct pool *, int);
void	 pool_allocator_free(struct pool *, void *);

#ifdef DDB
void	 pool_print_pagelist(struct pool_pagelist *,
	    int (*)(const char *, ...));
void	 pool_print1(struct pool *, const char *, int (*)(const char *, ...));
#endif

#define pool_sleep(pl) msleep(pl, &pl->pr_mtx, PSWP, pl->pr_wchan, 0)

static __inline int
phtree_compare(struct pool_item_header *a, struct pool_item_header *b)
{
	if (a->ph_page < b->ph_page)
		return (-1);
	else if (a->ph_page > b->ph_page)
		return (1);
	else
		return (0);
}

SPLAY_PROTOTYPE(phtree, pool_item_header, ph_node, phtree_compare);
SPLAY_GENERATE(phtree, pool_item_header, ph_node, phtree_compare);

/*
 * Return the pool page header based on page address.
 */
static __inline struct pool_item_header *
pr_find_pagehead(struct pool *pp, caddr_t page)
{
	struct pool_item_header *ph, tmp;

	if ((pp->pr_roflags & PR_PHINPAGE) != 0)
		return ((struct pool_item_header *)(page + pp->pr_phoffset));

	tmp.ph_page = page;
	ph = SPLAY_FIND(phtree, &pp->pr_phtree, &tmp);
	return ph;
}

/*
 * Remove a page from the pool.
 */
void
pr_rmpage(struct pool *pp, struct pool_item_header *ph,
     struct pool_pagelist *pq)
{

	/*
	 * If the page was idle, decrement the idle page count.
	 */
	if (ph->ph_nmissing == 0) {
#ifdef DIAGNOSTIC
		if (pp->pr_nidle == 0)
			panic("pr_rmpage: nidle inconsistent");
		if (pp->pr_nitems < pp->pr_itemsperpage)
			panic("pr_rmpage: nitems inconsistent");
#endif
		pp->pr_nidle--;
	}

	pp->pr_nitems -= pp->pr_itemsperpage;

	/*
	 * Unlink a page from the pool and release it (or queue it for release).
	 */
	LIST_REMOVE(ph, ph_pagelist);
	if ((pp->pr_roflags & PR_PHINPAGE) == 0)
		SPLAY_REMOVE(phtree, &pp->pr_phtree, ph);
	if (pq) {
		LIST_INSERT_HEAD(pq, ph, ph_pagelist);
	} else {
		pool_allocator_free(pp, ph->ph_page);
		if ((pp->pr_roflags & PR_PHINPAGE) == 0)
			pool_put(&phpool, ph);
	}
	pp->pr_npages--;
	pp->pr_npagefree++;

	pool_update_curpage(pp);
}

/*
 * Initialize the given pool resource structure.
 *
 * We export this routine to allow other kernel parts to declare
 * static pools that must be initialized before malloc() is available.
 */
void
pool_init(struct pool *pp, size_t size, u_int align, u_int ioff, int flags,
    const char *wchan, struct pool_allocator *palloc)
{
	int off, slack;

#ifdef MALLOC_DEBUG
	if ((flags & PR_DEBUG) && (ioff != 0 || align != 0))
		flags &= ~PR_DEBUG;
#endif
	/*
	 * Check arguments and construct default values.
	 */
	if (palloc == NULL)
		palloc = &pool_allocator_nointr;
	if (palloc->pa_pagesz == 0) {
		palloc->pa_pagesz = PAGE_SIZE;
		palloc->pa_pagemask = ~(palloc->pa_pagesz - 1);
		palloc->pa_pageshift = ffs(palloc->pa_pagesz) - 1;
	}

	if (align == 0)
		align = ALIGN(1);

	if (size < sizeof(struct pool_item))
		size = sizeof(struct pool_item);

	size = roundup(size, align);
#ifdef DIAGNOSTIC
	if (size > palloc->pa_pagesz)
		panic("pool_init: pool item size (%lu) too large",
		      (u_long)size);
#endif

	/*
	 * Initialize the pool structure.
	 */
	LIST_INIT(&pp->pr_emptypages);
	LIST_INIT(&pp->pr_fullpages);
	LIST_INIT(&pp->pr_partpages);
	pp->pr_curpage = NULL;
	pp->pr_npages = 0;
	pp->pr_minitems = 0;
	pp->pr_minpages = 0;
	pp->pr_maxpages = 8;
	pp->pr_roflags = flags;
	pp->pr_flags = 0;
	pp->pr_size = size;
	pp->pr_align = align;
	pp->pr_wchan = wchan;
	pp->pr_alloc = palloc;
	pp->pr_nitems = 0;
	pp->pr_nout = 0;
	pp->pr_hardlimit = UINT_MAX;
	pp->pr_hardlimit_warning = NULL;
	pp->pr_hardlimit_ratecap.tv_sec = 0;
	pp->pr_hardlimit_ratecap.tv_usec = 0;
	pp->pr_hardlimit_warning_last.tv_sec = 0;
	pp->pr_hardlimit_warning_last.tv_usec = 0;
	pp->pr_serial = ++pool_serial;
	if (pool_serial == 0)
		panic("pool_init: too much uptime");

	/*
	 * Decide whether to put the page header off page to avoid
	 * wasting too large a part of the page. Off-page page headers
	 * go on a hash table, so we can match a returned item
	 * with its header based on the page address.
	 * We use 1/16 of the page size as the threshold (XXX: tune)
	 */
	if (pp->pr_size < palloc->pa_pagesz/16) {
		/* Use the end of the page for the page header */
		pp->pr_roflags |= PR_PHINPAGE;
		pp->pr_phoffset = off = palloc->pa_pagesz -
		    ALIGN(sizeof(struct pool_item_header));
	} else {
		/* The page header will be taken from our page header pool */
		pp->pr_phoffset = 0;
		off = palloc->pa_pagesz;
		SPLAY_INIT(&pp->pr_phtree);
	}

	/*
	 * Alignment is to take place at `ioff' within the item. This means
	 * we must reserve up to `align - 1' bytes on the page to allow
	 * appropriate positioning of each item.
	 *
	 * Silently enforce `0 <= ioff < align'.
	 */
	pp->pr_itemoffset = ioff = ioff % align;
	pp->pr_itemsperpage = (off - ((align - ioff) % align)) / pp->pr_size;
	KASSERT(pp->pr_itemsperpage != 0);

	/*
	 * Use the slack between the chunks and the page header
	 * for "cache coloring".
	 */
	slack = off - pp->pr_itemsperpage * pp->pr_size;
	pp->pr_maxcolor = (slack / align) * align;
	pp->pr_curcolor = 0;

	pp->pr_nget = 0;
	pp->pr_nfail = 0;
	pp->pr_nput = 0;
	pp->pr_npagealloc = 0;
	pp->pr_npagefree = 0;
	pp->pr_hiwat = 0;
	pp->pr_nidle = 0;

	pp->pr_ipl = -1;
	mtx_init(&pp->pr_mtx, IPL_NONE);

	if (phpool.pr_size == 0) {
		pool_init(&phpool, sizeof(struct pool_item_header), 0, 0,
		    0, "phpool", NULL);
		pool_setipl(&phpool, IPL_HIGH);
	}

	/* Insert this into the list of all pools. */
	TAILQ_INSERT_TAIL(&pool_head, pp, pr_poollist);
}

void
pool_setipl(struct pool *pp, int ipl)
{
	pp->pr_ipl = ipl;
	mtx_init(&pp->pr_mtx, ipl);
}

/*
 * Decommission a pool resource.
 */
void
pool_destroy(struct pool *pp)
{
	struct pool_item_header *ph;

#ifdef DIAGNOSTIC
	if (pp->pr_nout != 0)
		panic("pool_destroy: pool busy: still out: %u", pp->pr_nout);
#endif

	/* Remove all pages */
	while ((ph = LIST_FIRST(&pp->pr_emptypages)) != NULL)
		pr_rmpage(pp, ph, NULL);
	KASSERT(LIST_EMPTY(&pp->pr_fullpages));
	KASSERT(LIST_EMPTY(&pp->pr_partpages));

	/* Remove from global pool list */
	TAILQ_REMOVE(&pool_head, pp, pr_poollist);
}

struct pool_item_header *
pool_alloc_item_header(struct pool *pp, caddr_t storage, int flags)
{
	struct pool_item_header *ph;

	if ((pp->pr_roflags & PR_PHINPAGE) != 0)
		ph = (struct pool_item_header *)(storage + pp->pr_phoffset);
	else {
		/*
		 * all callers use PR_NOWAIT except for one
		 * that comes directly from pool_get and carries
		 * potentiall PR_WAITOK given from the pool user.
		 * keep the intention and do not sleep on allocating
		 * the pool item headers instead the upper layer
		 * will sleep if necessary waiting for more memory.
		 */
		flags = (flags & ~PR_WAITOK) & PR_NOWAIT;
		ph = pool_get(&phpool, flags);
	}

	return (ph);
}

/*
 * Grab an item from the pool; must be called at appropriate spl level
 */
void *
pool_get(struct pool *pp, int flags)
{
	void *v;

	mtx_enter(&pp->pr_mtx);
	v = pool_do_get(pp, flags);
	mtx_leave(&pp->pr_mtx);
	if (v && pp->pr_ctor && pp->pr_ctor(pp->pr_arg, v, flags)) {
		mtx_enter(&pp->pr_mtx);
		pool_do_put(pp, v);
		mtx_leave(&pp->pr_mtx);
		v = NULL;
	}
	if (v) {
		pp->pr_nget++;
		if (flags & PR_ZERO)
			memset(v, 0, pp->pr_size);
	}
	return (v);
}

void *
pool_do_get(struct pool *pp, int flags)
{
	struct pool_item *pi;
	struct pool_item_header *ph;
	void *v;

#ifdef DIAGNOSTIC
	if ((flags & PR_WAITOK) != 0)
		splassert(IPL_NONE);
	if (pp->pr_ipl != -1)
		splassert(pp->pr_ipl);
#endif /* DIAGNOSTIC */

#ifdef MALLOC_DEBUG
	if (pp->pr_roflags & PR_DEBUG) {
		void *addr;

		addr = NULL;
		debug_malloc(pp->pr_size, M_DEBUG,
		    (flags & PR_WAITOK) ? M_WAITOK : M_NOWAIT, &addr);
		return (addr);
	}
#endif

startover:
	/*
	 * Check to see if we've reached the hard limit.  If we have,
	 * and we can wait, then wait until an item has been returned to
	 * the pool.
	 */
#ifdef DIAGNOSTIC
	if (__predict_false(pp->pr_nout > pp->pr_hardlimit))
		panic("pool_do_get: %s: crossed hard limit", pp->pr_wchan);
#endif
	if (__predict_false(pp->pr_nout == pp->pr_hardlimit)) {
		if ((flags & PR_WAITOK) && !(flags & PR_LIMITFAIL)) {
			/*
			 * XXX: A warning isn't logged in this case.  Should
			 * it be?
			 */
			pp->pr_flags |= PR_WANTED;
			pool_sleep(pp);
			goto startover;
		}

		/*
		 * Log a message that the hard limit has been hit.
		 */
		if (pp->pr_hardlimit_warning != NULL &&
		    ratecheck(&pp->pr_hardlimit_warning_last,
			      &pp->pr_hardlimit_ratecap))
			log(LOG_ERR, "%s\n", pp->pr_hardlimit_warning);

		pp->pr_nfail++;
		return (NULL);
	}

	/*
	 * The convention we use is that if `curpage' is not NULL, then
	 * it points at a non-empty bucket. In particular, `curpage'
	 * never points at a page header which has PR_PHINPAGE set and
	 * has no items in its bucket.
	 */
	if ((ph = pp->pr_curpage) == NULL) {
#ifdef DIAGNOSTIC
		if (pp->pr_nitems != 0) {
			printf("pool_do_get: %s: curpage NULL, nitems %u\n",
			    pp->pr_wchan, pp->pr_nitems);
			panic("pool_do_get: nitems inconsistent");
		}
#endif

		/*
		 * Call the back-end page allocator for more memory.
		 */
		v = pool_allocator_alloc(pp, flags);
		if (__predict_true(v != NULL))
			ph = pool_alloc_item_header(pp, v, flags);

		if (__predict_false(v == NULL || ph == NULL)) {
			if (v != NULL)
				pool_allocator_free(pp, v);

			if ((flags & PR_WAITOK) == 0) {
				pp->pr_nfail++;
				return (NULL);
			}

			/*
			 * Wait for items to be returned to this pool.
			 *
			 * XXX: maybe we should wake up once a second and
			 * try again?
			 */
			pp->pr_flags |= PR_WANTED;
			pool_sleep(pp);
			goto startover;
		}

		/* We have more memory; add it to the pool */
		pool_prime_page(pp, v, ph);
		pp->pr_npagealloc++;

		/* Start the allocation process over. */
		goto startover;
	}
	if (__predict_false((v = pi = TAILQ_FIRST(&ph->ph_itemlist)) == NULL)) {
		panic("pool_do_get: %s: page empty", pp->pr_wchan);
	}
#ifdef DIAGNOSTIC
	if (__predict_false(pp->pr_nitems == 0)) {
		printf("pool_do_get: %s: items on itemlist, nitems %u\n",
		    pp->pr_wchan, pp->pr_nitems);
		panic("pool_do_get: nitems inconsistent");
	}
#endif

#ifdef DIAGNOSTIC
	if (__predict_false(pi->pi_magic != PI_MAGIC)) {
		panic("pool_do_get(%s): free list modified: magic=%x; page %p;"
		       " item addr %p",
			pp->pr_wchan, pi->pi_magic, ph->ph_page, pi);
	}
#endif

	/*
	 * Remove from item list.
	 */
	TAILQ_REMOVE(&ph->ph_itemlist, pi, pi_list);
	pp->pr_nitems--;
	pp->pr_nout++;
	if (ph->ph_nmissing == 0) {
#ifdef DIAGNOSTIC
		if (__predict_false(pp->pr_nidle == 0))
			panic("pool_do_get: nidle inconsistent");
#endif
		pp->pr_nidle--;

		/*
		 * This page was previously empty.  Move it to the list of
		 * partially-full pages.  This page is already curpage.
		 */
		LIST_REMOVE(ph, ph_pagelist);
		LIST_INSERT_HEAD(&pp->pr_partpages, ph, ph_pagelist);
	}
	ph->ph_nmissing++;
	if (TAILQ_EMPTY(&ph->ph_itemlist)) {
#ifdef DIAGNOSTIC
		if (__predict_false(ph->ph_nmissing != pp->pr_itemsperpage)) {
			panic("pool_do_get: %s: nmissing inconsistent",
			    pp->pr_wchan);
		}
#endif
		/*
		 * This page is now full.  Move it to the full list
		 * and select a new current page.
		 */
		LIST_REMOVE(ph, ph_pagelist);
		LIST_INSERT_HEAD(&pp->pr_fullpages, ph, ph_pagelist);
		pool_update_curpage(pp);
	}

	/*
	 * If we have a low water mark and we are now below that low
	 * water mark, add more items to the pool.
	 */
	if (POOL_NEEDS_CATCHUP(pp) && pool_catchup(pp) != 0) {
		/*
		 * XXX: Should we log a warning?  Should we set up a timeout
		 * to try again in a second or so?  The latter could break
		 * a caller's assumptions about interrupt protection, etc.
		 */
	}
	return (v);
}

/*
 * Return resource to the pool; must be called at appropriate spl level
 */
void
pool_put(struct pool *pp, void *v)
{
	if (pp->pr_dtor)
		pp->pr_dtor(pp->pr_arg, v);
	mtx_enter(&pp->pr_mtx);
	pool_do_put(pp, v);
	mtx_leave(&pp->pr_mtx);
	pp->pr_nput++;
}

/*
 * Internal version of pool_put().
 */
void
pool_do_put(struct pool *pp, void *v)
{
	struct pool_item *pi = v;
	struct pool_item_header *ph;
	caddr_t page;

#ifdef MALLOC_DEBUG
	if (pp->pr_roflags & PR_DEBUG) {
		debug_free(v, M_DEBUG);
		return;
	}
#endif

	page = (caddr_t)((vaddr_t)v & pp->pr_alloc->pa_pagemask);

#ifdef DIAGNOSTIC
	if (pp->pr_ipl != -1)
		splassert(pp->pr_ipl);

	if (__predict_false(pp->pr_nout == 0)) {
		printf("pool %s: putting with none out\n",
		    pp->pr_wchan);
		panic("pool_do_put");
	}
#endif

	if (__predict_false((ph = pr_find_pagehead(pp, page)) == NULL)) {
		panic("pool_do_put: %s: page header missing", pp->pr_wchan);
	}

	/*
	 * Return to item list.
	 */
#ifdef DIAGNOSTIC
	pi->pi_magic = PI_MAGIC;
#endif
#ifdef DEBUG
	{
		int i, *ip = v;

		for (i = 0; i < pp->pr_size / sizeof(int); i++) {
			*ip++ = PI_MAGIC;
		}
	}
#endif

	TAILQ_INSERT_HEAD(&ph->ph_itemlist, pi, pi_list);
	ph->ph_nmissing--;
	pp->pr_nitems++;
	pp->pr_nout--;

	/* Cancel "pool empty" condition if it exists */
	if (pp->pr_curpage == NULL)
		pp->pr_curpage = ph;

	if (pp->pr_flags & PR_WANTED) {
		pp->pr_flags &= ~PR_WANTED;
		if (ph->ph_nmissing == 0)
			pp->pr_nidle++;
		wakeup(pp);
		return;
	}

	/*
	 * If this page is now empty, do one of two things:
	 *
	 *	(1) If we have more pages than the page high water mark,
	 *	    free the page back to the system.
	 *
	 *	(2) Otherwise, move the page to the empty page list.
	 *
	 * Either way, select a new current page (so we use a partially-full
	 * page if one is available).
	 */
	if (ph->ph_nmissing == 0) {
		pp->pr_nidle++;
		if (pp->pr_nidle > pp->pr_maxpages) {
			pr_rmpage(pp, ph, NULL);
		} else {
			LIST_REMOVE(ph, ph_pagelist);
			LIST_INSERT_HEAD(&pp->pr_emptypages, ph, ph_pagelist);
		}
		pool_update_curpage(pp);
	}

	/*
	 * If the page was previously completely full, move it to the
	 * partially-full list and make it the current page.  The next
	 * allocation will get the item from this page, instead of
	 * further fragmenting the pool.
	 */
	else if (ph->ph_nmissing == (pp->pr_itemsperpage - 1)) {
		LIST_REMOVE(ph, ph_pagelist);
		LIST_INSERT_HEAD(&pp->pr_partpages, ph, ph_pagelist);
		pp->pr_curpage = ph;
	}
}

/*
 * Add N items to the pool.
 */
int
pool_prime(struct pool *pp, int n)
{
	struct pool_item_header *ph;
	caddr_t cp;
	int newpages;

	mtx_enter(&pp->pr_mtx);
	newpages = roundup(n, pp->pr_itemsperpage) / pp->pr_itemsperpage;

	while (newpages-- > 0) {
		cp = pool_allocator_alloc(pp, PR_NOWAIT);
		if (__predict_true(cp != NULL))
			ph = pool_alloc_item_header(pp, cp, PR_NOWAIT);
		if (__predict_false(cp == NULL || ph == NULL)) {
			if (cp != NULL)
				pool_allocator_free(pp, cp);
			break;
		}

		pool_prime_page(pp, cp, ph);
		pp->pr_npagealloc++;
		pp->pr_minpages++;
	}

	if (pp->pr_minpages >= pp->pr_maxpages)
		pp->pr_maxpages = pp->pr_minpages + 1;	/* XXX */

	mtx_leave(&pp->pr_mtx);
	return (0);
}

/*
 * Add a page worth of items to the pool.
 *
 * Note, we must be called with the pool descriptor LOCKED.
 */
void
pool_prime_page(struct pool *pp, caddr_t storage, struct pool_item_header *ph)
{
	struct pool_item *pi;
	caddr_t cp = storage;
	unsigned int align = pp->pr_align;
	unsigned int ioff = pp->pr_itemoffset;
	int n;

#ifdef DIAGNOSTIC
	if (((u_long)cp & (pp->pr_alloc->pa_pagesz - 1)) != 0)
		panic("pool_prime_page: %s: unaligned page", pp->pr_wchan);
#endif

	/*
	 * Insert page header.
	 */
	LIST_INSERT_HEAD(&pp->pr_emptypages, ph, ph_pagelist);
	TAILQ_INIT(&ph->ph_itemlist);
	ph->ph_page = storage;
	ph->ph_nmissing = 0;
	if ((pp->pr_roflags & PR_PHINPAGE) == 0)
		SPLAY_INSERT(phtree, &pp->pr_phtree, ph);

	pp->pr_nidle++;

	/*
	 * Color this page.
	 */
	cp = (caddr_t)(cp + pp->pr_curcolor);
	if ((pp->pr_curcolor += align) > pp->pr_maxcolor)
		pp->pr_curcolor = 0;

	/*
	 * Adjust storage to apply aligment to `pr_itemoffset' in each item.
	 */
	if (ioff != 0)
		cp = (caddr_t)(cp + (align - ioff));

	/*
	 * Insert remaining chunks on the bucket list.
	 */
	n = pp->pr_itemsperpage;
	pp->pr_nitems += n;

	while (n--) {
		pi = (struct pool_item *)cp;

		KASSERT(((((vaddr_t)pi) + ioff) & (align - 1)) == 0);

		/* Insert on page list */
		TAILQ_INSERT_TAIL(&ph->ph_itemlist, pi, pi_list);
#ifdef DIAGNOSTIC
		pi->pi_magic = PI_MAGIC;
#endif
		cp = (caddr_t)(cp + pp->pr_size);
	}

	/*
	 * If the pool was depleted, point at the new page.
	 */
	if (pp->pr_curpage == NULL)
		pp->pr_curpage = ph;

	if (++pp->pr_npages > pp->pr_hiwat)
		pp->pr_hiwat = pp->pr_npages;
}

/*
 * Used by pool_get() when nitems drops below the low water mark.  This
 * is used to catch up pr_nitems with the low water mark.
 *
 * Note we never wait for memory here, we let the caller decide what to do.
 */
int
pool_catchup(struct pool *pp)
{
	struct pool_item_header *ph;
	caddr_t cp;
	int error = 0;

	while (POOL_NEEDS_CATCHUP(pp)) {
		/*
		 * Call the page back-end allocator for more memory.
		 */
		cp = pool_allocator_alloc(pp, PR_NOWAIT);
		if (__predict_true(cp != NULL))
			ph = pool_alloc_item_header(pp, cp, PR_NOWAIT);
		if (__predict_false(cp == NULL || ph == NULL)) {
			if (cp != NULL)
				pool_allocator_free(pp, cp);
			error = ENOMEM;
			break;
		}
		pool_prime_page(pp, cp, ph);
		pp->pr_npagealloc++;
	}

	return (error);
}

void
pool_update_curpage(struct pool *pp)
{

	pp->pr_curpage = LIST_FIRST(&pp->pr_partpages);
	if (pp->pr_curpage == NULL) {
		pp->pr_curpage = LIST_FIRST(&pp->pr_emptypages);
	}
}

void
pool_setlowat(struct pool *pp, int n)
{

	pp->pr_minitems = n;
	pp->pr_minpages = (n == 0)
		? 0
		: roundup(n, pp->pr_itemsperpage) / pp->pr_itemsperpage;

	mtx_enter(&pp->pr_mtx);
	/* Make sure we're caught up with the newly-set low water mark. */
	if (POOL_NEEDS_CATCHUP(pp) && pool_catchup(pp) != 0) {
		/*
		 * XXX: Should we log a warning?  Should we set up a timeout
		 * to try again in a second or so?  The latter could break
		 * a caller's assumptions about interrupt protection, etc.
		 */
	}
	mtx_leave(&pp->pr_mtx);
}

void
pool_sethiwat(struct pool *pp, int n)
{

	pp->pr_maxpages = (n == 0)
		? 0
		: roundup(n, pp->pr_itemsperpage) / pp->pr_itemsperpage;
}

int
pool_sethardlimit(struct pool *pp, u_int n, const char *warnmsg, int ratecap)
{
	int error = 0;

	if (n < pp->pr_nout) {
		error = EINVAL;
		goto done;
	}

	pp->pr_hardlimit = n;
	pp->pr_hardlimit_warning = warnmsg;
	pp->pr_hardlimit_ratecap.tv_sec = ratecap;
	pp->pr_hardlimit_warning_last.tv_sec = 0;
	pp->pr_hardlimit_warning_last.tv_usec = 0;

	/*
	 * In-line version of pool_sethiwat().
	 */
	pp->pr_maxpages = (n == 0 || n == UINT_MAX)
		? n
		: roundup(n, pp->pr_itemsperpage) / pp->pr_itemsperpage;

done:
	return (error);
}

void
pool_set_ctordtor(struct pool *pp, int (*ctor)(void *, void *, int),
    void (*dtor)(void *, void *), void *arg)
{
	pp->pr_ctor = ctor;
	pp->pr_dtor = dtor;
	pp->pr_arg = arg;
}
/*
 * Release all complete pages that have not been used recently.
 *
 * Returns non-zero if any pages have been reclaimed.
 */
int
pool_reclaim(struct pool *pp)
{
	struct pool_item_header *ph, *phnext;
	struct pool_pagelist pq;

	LIST_INIT(&pq);

	mtx_enter(&pp->pr_mtx);
	for (ph = LIST_FIRST(&pp->pr_emptypages); ph != NULL; ph = phnext) {
		phnext = LIST_NEXT(ph, ph_pagelist);

		/* Check our minimum page claim */
		if (pp->pr_npages <= pp->pr_minpages)
			break;

		KASSERT(ph->ph_nmissing == 0);

		/*
		 * If freeing this page would put us below
		 * the low water mark, stop now.
		 */
		if ((pp->pr_nitems - pp->pr_itemsperpage) <
		    pp->pr_minitems)
			break;

		pr_rmpage(pp, ph, &pq);
	}
	mtx_leave(&pp->pr_mtx);

	if (LIST_EMPTY(&pq))
		return (0);
	while ((ph = LIST_FIRST(&pq)) != NULL) {
		LIST_REMOVE(ph, ph_pagelist);
		pool_allocator_free(pp, ph->ph_page);
		if (pp->pr_roflags & PR_PHINPAGE)
			continue;
		pool_put(&phpool, ph);
	}

	return (1);
}

#ifdef DDB
#include <machine/db_machdep.h>
#include <ddb/db_interface.h>
#include <ddb/db_output.h>

/*
 * Diagnostic helpers.
 */
void
pool_printit(struct pool *pp, const char *modif, int (*pr)(const char *, ...))
{
	pool_print1(pp, modif, pr);
}

void
pool_print_pagelist(struct pool_pagelist *pl, int (*pr)(const char *, ...))
{
	struct pool_item_header *ph;
#ifdef DIAGNOSTIC
	struct pool_item *pi;
#endif

	LIST_FOREACH(ph, pl, ph_pagelist) {
		(*pr)("\t\tpage %p, nmissing %d\n",
		    ph->ph_page, ph->ph_nmissing);
#ifdef DIAGNOSTIC
		TAILQ_FOREACH(pi, &ph->ph_itemlist, pi_list) {
			if (pi->pi_magic != PI_MAGIC) {
				(*pr)("\t\t\titem %p, magic 0x%x\n",
				    pi, pi->pi_magic);
			}
		}
#endif
	}
}

void
pool_print1(struct pool *pp, const char *modif, int (*pr)(const char *, ...))
{
	struct pool_item_header *ph;
	int print_pagelist = 0;
	char c;

	while ((c = *modif++) != '\0') {
		if (c == 'p')
			print_pagelist = 1;
		modif++;
	}

	(*pr)("POOL %s: size %u, align %u, ioff %u, roflags 0x%08x\n",
	    pp->pr_wchan, pp->pr_size, pp->pr_align, pp->pr_itemoffset,
	    pp->pr_roflags);
	(*pr)("\talloc %p\n", pp->pr_alloc);
	(*pr)("\tminitems %u, minpages %u, maxpages %u, npages %u\n",
	    pp->pr_minitems, pp->pr_minpages, pp->pr_maxpages, pp->pr_npages);
	(*pr)("\titemsperpage %u, nitems %u, nout %u, hardlimit %u\n",
	    pp->pr_itemsperpage, pp->pr_nitems, pp->pr_nout, pp->pr_hardlimit);

	(*pr)("\n\tnget %lu, nfail %lu, nput %lu\n",
	    pp->pr_nget, pp->pr_nfail, pp->pr_nput);
	(*pr)("\tnpagealloc %lu, npagefree %lu, hiwat %u, nidle %lu\n",
	    pp->pr_npagealloc, pp->pr_npagefree, pp->pr_hiwat, pp->pr_nidle);

	if (print_pagelist == 0)
		return;

	if ((ph = LIST_FIRST(&pp->pr_emptypages)) != NULL)
		(*pr)("\n\tempty page list:\n");
	pool_print_pagelist(&pp->pr_emptypages, pr);
	if ((ph = LIST_FIRST(&pp->pr_fullpages)) != NULL)
		(*pr)("\n\tfull page list:\n");
	pool_print_pagelist(&pp->pr_fullpages, pr);
	if ((ph = LIST_FIRST(&pp->pr_partpages)) != NULL)
		(*pr)("\n\tpartial-page list:\n");
	pool_print_pagelist(&pp->pr_partpages, pr);

	if (pp->pr_curpage == NULL)
		(*pr)("\tno current page\n");
	else
		(*pr)("\tcurpage %p\n", pp->pr_curpage->ph_page);
}

void
db_show_all_pools(db_expr_t expr, int haddr, db_expr_t count, char *modif)
{
	struct pool *pp;
	char maxp[16];
	int ovflw;
	char mode;

	mode = modif[0];
	if (mode != '\0' && mode != 'a') {
		db_printf("usage: show all pools [/a]\n");
		return;
	}

	if (mode == '\0')
		db_printf("%-10s%4s%9s%5s%9s%6s%6s%6s%6s%6s%6s%5s\n",
		    "Name",
		    "Size",
		    "Requests",
		    "Fail",
		    "Releases",
		    "Pgreq",
		    "Pgrel",
		    "Npage",
		    "Hiwat",
		    "Minpg",
		    "Maxpg",
		    "Idle");
	else
		db_printf("%-10s %18s %18s\n",
		    "Name", "Address", "Allocator");

	TAILQ_FOREACH(pp, &pool_head, pr_poollist) {
		if (mode == 'a') {
			db_printf("%-10s %18p %18p\n", pp->pr_wchan, pp,
			    pp->pr_alloc);
			continue;
		}

		if (!pp->pr_nget)
			continue;

		if (pp->pr_maxpages == UINT_MAX)
			snprintf(maxp, sizeof maxp, "inf");
		else
			snprintf(maxp, sizeof maxp, "%u", pp->pr_maxpages);

#define PRWORD(ovflw, fmt, width, fixed, val) do {	\
	(ovflw) += db_printf((fmt),			\
	    (width) - (fixed) - (ovflw) > 0 ?		\
	    (width) - (fixed) - (ovflw) : 0,		\
	    (val)) - (width);				\
	if ((ovflw) < 0)				\
		(ovflw) = 0;				\
} while (/* CONSTCOND */0)

		ovflw = 0;
		PRWORD(ovflw, "%-*s", 10, 0, pp->pr_wchan);
		PRWORD(ovflw, " %*u", 4, 1, pp->pr_size);
		PRWORD(ovflw, " %*lu", 9, 1, pp->pr_nget);
		PRWORD(ovflw, " %*lu", 5, 1, pp->pr_nfail);
		PRWORD(ovflw, " %*lu", 9, 1, pp->pr_nput);
		PRWORD(ovflw, " %*lu", 6, 1, pp->pr_npagealloc);
		PRWORD(ovflw, " %*lu", 6, 1, pp->pr_npagefree);
		PRWORD(ovflw, " %*d", 6, 1, pp->pr_npages);
		PRWORD(ovflw, " %*d", 6, 1, pp->pr_hiwat);
		PRWORD(ovflw, " %*d", 6, 1, pp->pr_minpages);
		PRWORD(ovflw, " %*s", 6, 1, maxp);
		PRWORD(ovflw, " %*lu\n", 5, 1, pp->pr_nidle);
	}
}

int
pool_chk_page(struct pool *pp, const char *label, struct pool_item_header *ph)
{
	struct pool_item *pi;
	caddr_t page;
	int n;

	page = (caddr_t)((u_long)ph & pp->pr_alloc->pa_pagemask);
	if (page != ph->ph_page &&
	    (pp->pr_roflags & PR_PHINPAGE) != 0) {
		if (label != NULL)
			printf("%s: ", label);
		printf("pool(%p:%s): page inconsistency: page %p;"
		       " at page head addr %p (p %p)\n", pp,
			pp->pr_wchan, ph->ph_page,
			ph, page);
		return 1;
	}

	for (pi = TAILQ_FIRST(&ph->ph_itemlist), n = 0;
	     pi != NULL;
	     pi = TAILQ_NEXT(pi,pi_list), n++) {

#ifdef DIAGNOSTIC
		if (pi->pi_magic != PI_MAGIC) {
			if (label != NULL)
				printf("%s: ", label);
			printf("pool(%s): free list modified: magic=%x;"
			       " page %p; item ordinal %d;"
			       " addr %p (p %p)\n",
				pp->pr_wchan, pi->pi_magic, ph->ph_page,
				n, pi, page);
			panic("pool");
		}
#endif
		page =
		    (caddr_t)((u_long)pi & pp->pr_alloc->pa_pagemask);
		if (page == ph->ph_page)
			continue;

		if (label != NULL)
			printf("%s: ", label);
		printf("pool(%p:%s): page inconsistency: page %p;"
		       " item ordinal %d; addr %p (p %p)\n", pp,
			pp->pr_wchan, ph->ph_page,
			n, pi, page);
		return 1;
	}
	return 0;
}

int
pool_chk(struct pool *pp, const char *label)
{
	struct pool_item_header *ph;
	int r = 0;

	LIST_FOREACH(ph, &pp->pr_emptypages, ph_pagelist) {
		r = pool_chk_page(pp, label, ph);
		if (r) {
			goto out;
		}
	}
	LIST_FOREACH(ph, &pp->pr_fullpages, ph_pagelist) {
		r = pool_chk_page(pp, label, ph);
		if (r) {
			goto out;
		}
	}
	LIST_FOREACH(ph, &pp->pr_partpages, ph_pagelist) {
		r = pool_chk_page(pp, label, ph);
		if (r) {
			goto out;
		}
	}

out:
	return (r);
}
#endif

/*
 * We have three different sysctls.
 * kern.pool.npools - the number of pools.
 * kern.pool.pool.<pool#> - the pool struct for the pool#.
 * kern.pool.name.<pool#> - the name for pool#.
 */
int
sysctl_dopool(int *name, u_int namelen, char *where, size_t *sizep)
{
	struct pool *pp, *foundpool = NULL;
	size_t buflen = where != NULL ? *sizep : 0;
	int npools = 0, s;
	unsigned int lookfor;
	size_t len;

	switch (*name) {
	case KERN_POOL_NPOOLS:
		if (namelen != 1 || buflen != sizeof(int))
			return (EINVAL);
		lookfor = 0;
		break;
	case KERN_POOL_NAME:
		if (namelen != 2 || buflen < 1)
			return (EINVAL);
		lookfor = name[1];
		break;
	case KERN_POOL_POOL:
		if (namelen != 2 || buflen != sizeof(struct pool))
			return (EINVAL);
		lookfor = name[1];
		break;
	default:
		return (EINVAL);
	}

	s = splvm();

	TAILQ_FOREACH(pp, &pool_head, pr_poollist) {
		npools++;
		if (lookfor == pp->pr_serial) {
			foundpool = pp;
			break;
		}
	}

	splx(s);

	if (*name != KERN_POOL_NPOOLS && foundpool == NULL)
		return (ENOENT);

	switch (*name) {
	case KERN_POOL_NPOOLS:
		return copyout(&npools, where, buflen);
	case KERN_POOL_NAME:
		len = strlen(foundpool->pr_wchan) + 1;
		if (*sizep < len)
			return (ENOMEM);
		*sizep = len;
		return copyout(foundpool->pr_wchan, where, len);
	case KERN_POOL_POOL:
		return copyout(foundpool, where, buflen);
	}
	/* NOTREACHED */
	return (0); /* XXX - Stupid gcc */
}

/*
 * Pool backend allocators.
 *
 * Each pool has a backend allocator that handles allocation, deallocation
 */
void	*pool_page_alloc_oldnointr(struct pool *, int);
void	pool_page_free_oldnointr(struct pool *, void *);
void	*pool_page_alloc(struct pool *, int);
void	pool_page_free(struct pool *, void *);

/*
 * safe for interrupts, name preserved for compat this is the default
 * allocator
 */
struct pool_allocator pool_allocator_nointr = {
	pool_page_alloc, pool_page_free, 0,
};

/*
 * XXX - we have at least three different resources for the same allocation
 *  and each resource can be depleted. First we have the ready elements in
 *  the pool. Then we have the resource (typically a vm_map) for this
 *  allocator, then we have physical memory. Waiting for any of these can
 *  be unnecessary when any other is freed, but the kernel doesn't support
 *  sleeping on multiple addresses, so we have to fake. The caller sleeps on
 *  the pool (so that we can be awakened when an item is returned to the pool),
 *  but we set PA_WANT on the allocator. When a page is returned to
 *  the allocator and PA_WANT is set pool_allocator_free will wakeup all
 *  sleeping pools belonging to this allocator. (XXX - thundering herd).
 *  We also wake up the allocator in case someone without a pool (malloc)
 *  is sleeping waiting for this allocator.
 */

void *
pool_allocator_alloc(struct pool *pp, int flags)
{
	boolean_t waitok = (flags & PR_WAITOK) ? TRUE : FALSE;
	void *v;

	if (waitok)
		mtx_leave(&pp->pr_mtx);
	v = pp->pr_alloc->pa_alloc(pp, flags);
	if (waitok)
		mtx_enter(&pp->pr_mtx);

	return (v);
}

void
pool_allocator_free(struct pool *pp, void *v)
{
	struct pool_allocator *pa = pp->pr_alloc;

	(*pa->pa_free)(pp, v);
}

void *
pool_page_alloc(struct pool *pp, int flags)
{
	boolean_t waitok = (flags & PR_WAITOK) ? TRUE : FALSE;

	return (uvm_km_getpage(waitok));
}

void
pool_page_free(struct pool *pp, void *v)
{

	uvm_km_putpage(v);
}
