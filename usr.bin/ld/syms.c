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
static const char rcsid[] = "$ABSD: syms.c,v 1.7 2010/06/01 12:59:56 mickey Exp $";
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

static inline int
symcmp(struct symlist *a, struct symlist *b)
{
	return strcmp(a->sl_name, b->sl_name);
}

SPLAY_HEAD(symtree, symlist) undsyms = SPLAY_INITIALIZER(undsyms),
    defsyms = SPLAY_INITIALIZER(defsyms);

SPLAY_PROTOTYPE(symtree, symlist, sl_node, symcmp);

SPLAY_GENERATE(symtree, symlist, sl_node, symcmp);

struct symlist *
sym_undef(const char *name)
{
	struct symlist *sym;

	if (!(sym = calloc(1, sizeof *sym)))
		err(1, "calloc");

	if (!(sym->sl_name = strdup(name)))
		err(1, "strdup");

	SPLAY_INSERT(symtree, &undsyms, sym);
	return sym;
}

struct symlist *
sym_isundef(const char *name)
{
	struct symlist key;

	bzero(&key, sizeof key);
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
	SPLAY_REMOVE(symtree, &defsyms, sym);
	if (sym->sl_sect)
		TAILQ_REMOVE(&sym->sl_sect->os_syms, sym, sl_entry);
	sym->sl_sect = os;
	if (esym)
		memcpy(&sym->sl_elfsym, esym, sizeof sym->sl_elfsym);
	TAILQ_INSERT_TAIL(&os->os_syms, sym, sl_entry);
	SPLAY_INSERT(symtree, &defsyms, sym);
	return sym;
}

struct symlist *
sym_add(const char *name, struct section *os, void *esym)
{
	struct symlist *sym;

	if (!(sym = calloc(1, sizeof *sym)))
		err(1, "calloc");

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
sym_isdefined(const char *name, struct section *os)
{
	struct symlist key, *sym;

	bzero(&key, sizeof key);
	key.sl_name = name;
	if (!(sym = SPLAY_FIND(symtree, &defsyms, &key)) && os) {
		key.sl_sect = os;
		sym = SPLAY_FIND(symtree, &defsyms, &key);
	}

	return sym;
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
 * these are comparison functions used to sort relocation
 * array in elf_loadrelocs()
 */
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

	if ((neworder = calloc(1, sizeof *neworder)) == NULL)
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
