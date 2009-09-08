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
static const char rcsid[] = "$ABSD: ld2.c,v 1.1 2009/09/04 09:34:05 mickey Exp $";
#endif

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf_abi.h>
#include <elfuncs.h>
#include <a.out.h>
#include <nlist.h>
#include <errno.h>
#include <err.h>

#include "ld.h"

#if ELFSIZE == 32
#define	ldnarch		ldnarch32
#define	ldmap		ldmap32
#define	ldmap_obj	ldmap32_obj
#define	ldload		ldload32
#define	ldarch		ldarch32
#define	elf_fix_header	elf32_fix_header
#define	elf_ld_chkhdr	elf32_ld_chkhdr
#define	elf2nlist	elf32_2nlist
#define	elf_load_shdrs	elf32_load_shdrs
#define	elf_fix_shdrs	elf32_fix_shdrs
#define	elf_loadrelocs	elf32_loadrelocs
#define	elf_symload	elf32_symload
#define	elf_symadd	elf32_symadd
#define	elf_objadd	elf32_objadd
#elif ELFSIZE == 64
#define	ldnarch		ldnarch64
#define	ldmap		ldmap64
#define	ldmap_obj	ldmap64_obj
#define	ldload		ldload64
#define	ldarch		ldarch64
#define	elf_fix_header	elf64_fix_header
#define	elf_ld_chkhdr	elf64_ld_chkhdr
#define	elf2nlist	elf64_2nlist
#define	elf_load_shdrs	elf64_load_shdrs
#define	elf_fix_shdrs	elf64_fix_shdrs
#define	elf_loadrelocs	elf64_loadrelocs
#define	elf_symload	elf64_symload
#define	elf_symadd	elf64_symadd
#define	elf_objadd	elf64_objadd
#else
#error "Unsupported ELF class"
#endif

struct ldarch {
	int	la_mach;
	const struct ldorder *la_order;
} ldarch[] = {
#if ELFSIZE == 32
/*	{ EM_VAX, vax_order }, */
	{ EM_386, i386_order },
/*	{ EM_PARISC, hppa_order }, */
/*	{ EM_MIPS, mips_order  }, */
/*	{ EM_PPC, ppc_order }, */
/*	{ EM_SPARC, sparc_order }, */
/*	{ EM_SH, sh_order }, */
/*	{ EM_ARM, arm_order }, */
/*	{ EM_68K, m68k_order }, */
#elif ELFSIZE == 64
/*	{ EM_ALPHA, alpha_order }, */
/*	{ EM_AMD64, amd64_order }, */
/*	{ EM_PARISC64, hppa64_order }, */
/*	{ EM_MIPS64, mips64_order }, */
/*	{ EM_PPC64, ppc64_order }, */
/*	{ EM_SPARC64, sparc64_order }, */
#endif
};
int ldnarch = sizeof(ldarch)/sizeof(ldarch[0]);

const struct ldorder *
ldmap(void)
{
	TAILQ_HEAD(, ldorder) headorder;
	const struct ldorder *order;
	struct ldorder *neworder;
	u_long point;
	int i;

	for (i = 0; i < ldnarch; i++)
		if (ldarch[i].la_mach == machine)
			break;
	if (i >= ldnarch) {
		warn("unknown machine %d", machine);
		return NULL;
	}

	point = 0;
	neworder = NULL;
	TAILQ_INIT(&headorder);
	order = ldarch[i].la_order;
	for (i = 0; order[i].ldo_order != ldo_kaput; i++) {
		switch (order[i].ldo_order) {
		case ldo_option:
			/* TODO parse options */
			break;

		case ldo_expr:
			/* TODO calculate new expression */
			break;

		case ldo_section:
			if ((neworder = calloc(1, sizeof(*neworder))) == NULL)
				err(1, "calloc");

			TAILQ_INIT(&neworder->ldo_seclst);
			neworder->ldo_type = order[i].ldo_type;
			neworder->ldo_name = order[i].ldo_name;
			neworder->ldo_start = neworder->ldo_addr = point;
			/* roll thru all objs and map the ordered sections */
			if (obj_foreach(ldmap_obj, neworder)) {
				free(neworder);
				return NULL;
			}
			TAILQ_INSERT_TAIL(&headorder, neworder, ldo_entry);
			point = neworder->ldo_addr;
			break;

		case ldo_symbol:
			/* TODO (un)define the bloody symbol */
			break;

		case ldo_kaput:
			err(1, "kaputziner");
		}
	}

	if (printmap) {
		TAILQ_FOREACH(order, &headorder, ldo_entry) {
			struct section *sect;

			printf("%-16s0x%08lx\t0x%lx\n", order->ldo_name,
			    order->ldo_start,
			    order->ldo_addr - order->ldo_start);

			TAILQ_FOREACH(sect, &order->ldo_seclst, os_entry) {
				struct symlist *sym;
				struct objlist *ol;
				int head = 0;

				ol = sect->os_obj;
				TAILQ_FOREACH(sym, &ol->ol_syms, sl_entry) {
#if ELFSIZE == 32
					Elf_Sym *esym = &sym->sl_elfsym.sym32;
#else
					Elf_Sym *esym = &sym->sl_elfsym.sym64;
#endif
					if (esym->st_shndx != sect->os_no)
						continue;

					if (!head) {
						printf("%-16s  %8s\t%s\n", "",
						    "", ol->ol_name);
						head++;
					}

					printf("%-16s0x%08lx\t\t%s\n", "",
					    (u_long)esym->st_value,
					    sym->sl_name);
				}
			}  
		}
	}

	if (sym_undcheck())
		return NULL;

	return TAILQ_FIRST(&headorder);
}

int
ldmap_obj(struct objlist *ol, void *v)
{
	struct ldorder *neworder = v;
	Elf_Shdr *shdr = ol->ol_sects;
	struct section *sect = ol->ol_sections;
	struct symlist *sym;
	char *sname;
	int i, n;

#if ELFSIZE == 32
	n = ol->ol_hdr.elf32.e_shnum;
#else
	n = ol->ol_hdr.elf64.e_shnum;
#endif
	for (i = 0; i < n; i++, sect++, shdr++) {
		sname = ol->ol_snames + shdr->sh_name;
		if (strcmp(sname, neworder->ldo_name))
			continue;

		shdr->sh_addr = neworder->ldo_addr;
		neworder->ldo_addr += shdr->sh_size;
/* TODO align addr to shdr->sh_addralign */

		/* assign addrs to all symbols in this section */
		TAILQ_FOREACH(sym, &ol->ol_syms, sl_entry) {
#if ELFSIZE == 32
			Elf32_Sym *esym = &sym->sl_elfsym.sym32;
#else
			Elf64_Sym *esym = &sym->sl_elfsym.sym64;
#endif
			if (esym->st_shndx != i)
				continue;

			esym->st_value += shdr->sh_addr;
		}

		TAILQ_INSERT_TAIL(&neworder->ldo_seclst, sect, os_entry);
		break;
	}

	return 0;
}

/* TODO tail-recurse into the neworder */
int
ldload(const char *name, const struct ldorder *order)
{
	if (!order || errors)
		return 1;

	/* TODO generate phdr */

	/* TODO write out the sections */

	/* rejoice */
	return 0;
}

int
elf_loadrelocs(struct objlist *ol, struct section *s, Elf_Shdr *shdr,
    FILE *fp, off_t foff)
{
	off_t off;
	struct relist *r;
	Elf_Rel rel;
	Elf_RelA rela;
	int i, n, sz;

	off = ftello(fp);
	if (fseeko(fp, foff + shdr->sh_offset, SEEK_SET) < 0)
		err(1, "fseeko: %s", ol->ol_path);

	sz = shdr->sh_type == SHT_REL? sizeof rel : sizeof rela;
	n = shdr->sh_size / sz;
	if (!(r = calloc(n, sizeof *r)))
		err(1, "calloc");

	s->os_rels = r;
	s->os_nrls = n;
	if (shdr->sh_type == SHT_REL) {
		for (i = 0; i < n; i++) {
			if (fread(&rel, sizeof rel, 1, fp) != 1)
				err(1, "fread: %s", ol->ol_path);

			r->rl_sym = NULL;
			r->rl_addend = 0;
#if ELFSIZE == 32
			r->rl_addr = swap32(rel.r_offset);
			rel.r_info = swap32(rel.r_info);
#else
			r->rl_addr = swap64(rel.r_offset);
			rel.r_info = swap64(rel.r_info);
#endif
			r->rl_symidx = ELF_R_SYM(rel.r_info);
			r->rl_type = ELF_R_TYPE(rel.r_info);
		}
	} else {
		for (i = 0; i < n; i++) {
			if (fread(&rela, sizeof rela, 1, fp) != 1)
				err(1, "fread: %s", ol->ol_path);

			r->rl_sym = NULL;
#if ELFSIZE == 32
			r->rl_addr = swap32(rela.r_offset);
			r->rl_addend = swap32(rela.r_addend);
			rela.r_info = swap32(rela.r_info);
#else
			r->rl_addr = swap64(rela.r_offset);
			r->rl_addend = swap64(rela.r_addend);
			rela.r_info = swap64(rela.r_info);
#endif
			r->rl_symidx = ELF_R_SYM(rela.r_info);
			r->rl_type = ELF_R_TYPE(rela.r_info);
		}
	}

	if (fseeko(fp, off, SEEK_SET) < 0)
		err(1, "fseeko: %s", ol->ol_path);

	if (s->os_nrls > ELF_RELSORT)
		qsort(s->os_rels, s->os_nrls, sizeof *s->os_rels,
		    rel_symcmp);
	return 0;
}

int
elf_symadd(struct elf_symtab *es, int is, void *vs, void *v)
{
	struct nlist nl;
	struct objlist *ol = v;
	Elf_Sym *esym = vs;
	struct symlist *isdef, *sym;

	bzero(&nl, sizeof nl);
	elf2nlist(esym, es->ehdr, es->shdr, es->shstr, &nl);
	if (!nl.n_un.n_strx)
		return 0;

	nl.n_un.n_name = es->stab + nl.n_un.n_strx;
	if (!(nl.n_type & N_EXT) || (nl.n_type & N_TYPE) == N_FN ||
	    (nl.n_type & N_TYPE) == N_SIZE)
		return 0;

	isdef = sym_isdefined(nl.n_un.n_name);
	sym = sym_isundef(nl.n_un.n_name);
	if ((nl.n_type & N_TYPE) == N_UNDF) {
		if (sym || isdef)
			return rel_fixsyms(ol, isdef? isdef : sym, is);
		sym = sym_undef(nl.n_un.n_name);
		sym->sl_obj = ol;
	} else if (isdef) {
		sym = isdef;
		warnx("%s: multiply defined, previously in %s",
		    nl.n_un.n_name, ol->ol_name);
		errors++;
	} else {
		if (sym)
			sym_define(sym, ol, vs);
		else
			sym = sym_add(nl.n_un.n_name, ol, vs);
	}

	return rel_fixsyms(ol, sym, is);
}

int
elf_objadd(struct objlist *ol, FILE *fp, off_t foff)
{
	struct section *s, *se;
	struct elf_symtab es;
	Elf_Ehdr *eh;
	Elf_Shdr *shdr;
	int i, n;

#if ELFSIZE == 32
	eh = &ol->ol_hdr.elf32;
#else
	eh = &ol->ol_hdr.elf64;
#endif
	elf_fix_header(eh);
	if (elf_ld_chkhdr(ol->ol_name, eh, ET_REL,
	    &machine, &elfclass, &endian))
		return 1;

	if (!(shdr = elf_load_shdrs(ol->ol_name, fp, foff, eh)))
		return 1;
	elf_fix_shdrs(eh, shdr);

	n = ol->ol_nsect = eh->e_shnum;
	if (!(ol->ol_sections = calloc(n, sizeof(struct section))))
		err(1, "malloc");

	/* scan thru the section list looking for progbits and relocs */
	for (i = 0; i < n; i++) {
		s = &ol->ol_sections[i];
		s->os_no = i;
		s->os_sect = &shdr[i];
		s->os_obj = ol;

		if (shdr[i].sh_type != SHT_PROGBITS)
			continue;

		if (i + 1 >= n)
			continue;

		if (shdr[i + 1].sh_type != SHT_RELA &&
		    shdr[i + 1].sh_type != SHT_REL)
			continue;

		if (elf_loadrelocs(ol, s, &shdr[i + 1], fp, foff))
			continue;
	}

	es.name = ol->ol_name;
	es.ehdr = eh;
	es.shdr = shdr;
	es.shstr = NULL;

	if (elf_symload(&es, fp, foff, elf_symadd, ol))
		return 1;
	free(es.stab);

	ol->ol_sects = es.shdr;
	ol->ol_snames = es.shstr;

	/* (re)sort the relocs by the address for the loader's pleasure */
	for (s = ol->ol_sections, se = s + n; s < se; s++)
		if (s->os_nrls)
			qsort(s->os_rels, s->os_nrls, sizeof *s->os_rels,
			    rel_addrcmp);
	return 0;
}

int
elf_ld_chkhdr(const char *path, Elf_Ehdr *eh, int type, int *mach,
    int *class, int *endian)
{

	if (eh->e_type != type) {
		warnx("%s: not a relocatable file", path);
		return EFTYPE;
	}

	if (*mach == EM_NONE) {
		*mach = eh->e_machine;
		*class = eh->e_ident[EI_CLASS];
		*endian = eh->e_ident[EI_DATA];
		return 0;
	}

	if (*mach != eh->e_machine ||
	    *class != eh->e_ident[EI_CLASS] ||
	    *endian != eh->e_ident[EI_DATA]) {
		warnx("%s: incompatible file type", path);
		return EFTYPE;
	}

	return 0;
}
