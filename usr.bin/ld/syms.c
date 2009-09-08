/*
 * Copyright (c) 2009 Michael Shalayeff
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
static const char rcsid[] = "$ABSD: syms.c,v 1.1 2009/09/04 09:34:05 mickey Exp $";
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
sym_define(struct symlist *sym, struct objlist *ol, void *esym)
{
	SPLAY_REMOVE(symtree, &undsyms, sym);
	sym->sl_obj = ol;
	memcpy(&sym->sl_elfsym, esym, sizeof sym->sl_elfsym);
	SPLAY_INSERT(symtree, &defsyms, sym);
	TAILQ_INSERT_TAIL(&ol->ol_syms, sym, sl_entry);
	return sym;
}

struct symlist *
sym_add(const char *name, struct objlist *ol, void *esym)
{
	struct symlist *sym;

	if (!(sym = calloc(1, sizeof *sym)))
		err(1, "malloc");

	if (!(sym->sl_name = strdup(name)))
		err(1, "strdup");

	sym->sl_obj = ol;
	memcpy(&sym->sl_elfsym, esym, sizeof sym->sl_elfsym);
	SPLAY_INSERT(symtree, &defsyms, sym);
	TAILQ_INSERT_TAIL(&ol->ol_syms, sym, sl_entry);
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

	if (!SPLAY_EMPTY(&undsyms)) {
		SPLAY_FOREACH(sym, symtree, &undsyms) {
			if (sym->sl_obj)
				warnx("%s: undefined, first used in %s",
				    sym->sl_name, sym->sl_obj->ol_name);
			else
				warnx("%s: undefined", sym->sl_name);
		}
		return -1;
	}

	return 0;
}

/*
 * scan all sections in this object and fix all relocs
 * referencing this symbol
 */
int
rel_fixsyms(struct objlist *ol, struct symlist *sym, int is)
{
	struct section *s;
	int i;

	for (i = 0, s = ol->ol_sections; i < ol->ol_nsect; s++, i++) {
		struct relist *r, *er;

		if (!s->os_nrls)
			continue;

		if (s->os_nrls > ELF_RELSORT) {
			struct relist rk, *r0;

			rk.rl_symidx = is;
			if (!(r0 = bsearch(&rk, s->os_rels, s->os_nrls,
			    sizeof rk, rel_symcmp)))
				continue;

			for (r = r0, er = s->os_rels; er <= r; r--)
				if (r->rl_symidx == is)
					r->rl_sym = sym;
				else
					break;

			for (r0++, er = s->os_rels + s->os_nrls; r0 < er; r0++)
				if (r->rl_symidx == is)
					r->rl_sym = sym;
				else
					break;

		} else
			for (r = s->os_rels, er = r + s->os_nrls; r < er; r++)
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
