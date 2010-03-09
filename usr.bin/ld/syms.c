/*
 * Copyright (c) 2009,2010 Michael Shalayeff
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

#ifndef lint
static const char rcsid[] = "$ABSD: syms.c,v 1.5 2010/01/10 05:56:01 mickey Exp $";
#endif

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf_abi.h>
#include <elfuncs.h>
#include <a.out.h>
#include <errno.h>
#include <err.h>

#include "ld.h"

SPLAY_HEAD(symtree, symlist)
    undsyms = SPLAY_INITIALIZER(undsyms),
    defsyms = SPLAY_INITIALIZER(defsyms);

static inline int
symcmp(struct symlist *a, struct symlist *b)
{
	return strcmp(a->sl_name, b->sl_name);
}

SPLAY_PROTOTYPE(symtree, symlist, sl_node, symcmp);

SPLAY_GENERATE(symtree, symlist, sl_node, symcmp);

/* TODO we have to deal with duplicate global names as in global statics
   perhaps by the means of using objlist* as an additional key */

struct symlist *
sym_undef(const char *name)
{
	struct symlist *sym;

	if (!(sym = calloc(1, sizeof *sym)))
		err(1, "malloc");

	if (!(sym->sl_name = strdup(name)))
		err(1, "strdup");

	SPLAY_INSERT(symtree, &undsyms, sym);
	return sym;
}

struct symlist *
sym_isundef(const char *name)
{
	struct symlist key;

	key.sl_name = name;
	return SPLAY_FIND(symtree, &undsyms, &key);
}

struct symlist *
sym_define(struct symlist *sym, struct section *os, void *esym)
{
	SPLAY_REMOVE(symtree, &undsyms, sym);
	sym->sl_sect = os;
	memcpy(&sym->sl_elfsym, esym, sizeof sym->sl_elfsym);
	SPLAY_INSERT(symtree, &defsyms, sym);
	/* ABS symbols have no section */
	if (os)
		TAILQ_INSERT_TAIL(&os->os_syms, sym, sl_entry);
	return sym;
}

struct symlist *
sym_redef(struct symlist *sym, struct section *os, void *esym)
{
	TAILQ_REMOVE(&sym->sl_sect->os_syms, sym, sl_entry);
	sym->sl_sect = os;
	memcpy(&sym->sl_elfsym, esym, sizeof sym->sl_elfsym);
	TAILQ_INSERT_TAIL(&os->os_syms, sym, sl_entry);
	return sym;
}

struct symlist *
sym_add(const char *name, struct section *os, void *esym)
{
	struct symlist *sym;

	if (!(sym = calloc(1, sizeof *sym)))
		err(1, "malloc");

	if (!(sym->sl_name = strdup(name)))
		err(1, "strdup");

	sym->sl_sect = os;
	memcpy(&sym->sl_elfsym, esym, sizeof sym->sl_elfsym);
	SPLAY_INSERT(symtree, &defsyms, sym);
	/* ABS symbols have no section */
	if (os)
		TAILQ_INSERT_TAIL(&os->os_syms, sym, sl_entry);
	return sym;
}

struct symlist *
sym_isdefined(const char *name)
{
	struct symlist key;

	key.sl_name = name;
	return SPLAY_FIND(symtree, &defsyms, &key);
}

int
sym_undcheck(void)
{
	struct symlist *sym;
	int err = 0;

	SPLAY_FOREACH(sym, symtree, &undsyms) {
		if (sym->sl_sect)
			warnx("%s: undefined, first used in %s",
			    sym->sl_name,
			    sym->sl_sect->os_obj->ol_name);
		else
			warnx("%s: undefined", sym->sl_name);
		err = -1;
	}

/* TODO check that all relocs got syms resolved! */

	return err;
}

void
sym_scan(const struct ldorder *order, int (*of)(const struct ldorder *, void *),
    int (*sf)(const struct ldorder *, const struct section *, struct symlist *,
    void *), void *v)
{
	for(; order != TAILQ_END(NULL); order = TAILQ_NEXT(order, ldo_entry)) {
		struct section *os;

		if (of && (*of)(order, v))
			return;

		TAILQ_FOREACH(os, &order->ldo_seclst, os_entry) {
			struct symlist *sym;

			TAILQ_FOREACH(sym, &os->os_syms, sl_entry)
				if (sf && (*sf)(order, os, sym, v))
					return;
		}
	}
}

/*
 * scan all sections in this object and fix all relocs
 * referencing this symbol
 */
int
rel_fixsyms(struct objlist *ol, struct symlist *sym, int is)
{
	struct section *os;
	int i;

	for (i = 0, os = ol->ol_sections; i < ol->ol_nsect; os++, i++) {
		struct relist *r, *er;

		if (!os->os_nrls)
			continue;

		if (os->os_nrls > ELF_RELSORT) {
			struct relist rk, *r0;

			rk.rl_symidx = is;
			if (!(r0 = bsearch(&rk, os->os_rels, os->os_nrls,
			    sizeof rk, rel_symcmp)))
				continue;

			for (r = r0, er = os->os_rels; er <= r; r--)
				if (r->rl_symidx == is)
					r->rl_sym = sym;
				else
					break;

			r0++;
			for (er = os->os_rels + os->os_nrls; r0 < er; r0++)
				if (r0->rl_symidx == is)
					r0->rl_sym = sym;
				else
					break;

		} else
			for (r = os->os_rels, er = r + os->os_nrls; r < er; r++)
				if (r->rl_symidx == is)
					r->rl_sym = sym;
	}

	return 0;
}

/*
 * these are comparison functions used to sort relocation
 * array in elf_loadrelocs()
 */
int
rel_symcmp(const void *a0, const void *b0)
{
	const struct relist *a = a0, *b = b0;

	return a->rl_symidx - b->rl_symidx;
}

int
rel_addrcmp(const void *a0, const void *b0)
{
	const struct relist *a = a0, *b = b0;

	if (a->rl_addr < b->rl_addr)
		return -1;
	else if (a->rl_addr > b->rl_addr)
		return 1;
	else
		return 0;
}

/*
 * allocate new order piece cloning from the templar
 */
struct ldorder *
order_clone(const struct ldarch *lda, const struct ldorder *order)
{
	struct ldorder *neworder;

	if ((neworder = calloc(1, sizeof(*neworder))) == NULL)
		err(1, "calloc");

	TAILQ_INIT(&neworder->ldo_seclst);
	neworder->ldo_order = order->ldo_order;
	neworder->ldo_name = order->ldo_name;
	neworder->ldo_type = order->ldo_type;
	neworder->ldo_flags = order->ldo_flags;
	neworder->ldo_shflags = order->ldo_shflags;
	neworder->ldo_arch = lda;

	return neworder;
}

int
order_printmap(const struct ldorder *order, void *v)
{
	switch (order->ldo_order) {
	case ldo_expr:
		break;

	case ldo_symbol:
		printf("%-16s0x%08llx\n", order->ldo_name, order->ldo_start);
		break;

	default:
		printf("%-16s0x%08llx\t0x%llx\n", order->ldo_name,
		    order->ldo_start, order->ldo_addr - order->ldo_start);
	}

	return 0;
}

int
randombit(void)
{
	static uint32_t b;
	static int i;

	if (i == 0) {
		b = arc4random();
		i = 31;
	}

	return b & (1 << i--);
}
