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
static const char rcsid[] = "$ABSD: ld2.c,v 1.4 2009/09/08 22:43:35 mickey Exp $";
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

/* some of these also duplicate libelf/elf.c */
#if ELFSIZE == 32
#define	ELF_HDR(h)	((h).elf32)
#define	ELF_SYM(h)	((h).sym32)
#define	ldnote		ldnote32
#define	ldmap		ldmap32
#define	ldmap_obj	ldmap32_obj
#define	ldload		ldload32
#define	ldloadasect	ldloadasect32
#define	elf_ld_chkhdr	elf32_ld_chkhdr
#define	elf_fix_header	elf32_fix_header
#define	elf_fix_note	elf32_fix_note
#define	elf_fix_phdrs	elf32_fix_phdrs
#define	elf_fix_shdrs	elf32_fix_shdrs
#define	elf_fix_rel	elf32_fix_rel
#define	elf_fix_rela	elf32_fix_rela
#define	elf2nlist	elf32_2nlist
#define	elf_load_shdrs	elf32_load_shdrs
#define	elf_fix_shdrs	elf32_fix_shdrs
#define	elf_symload	elf32_symload
#define	elf_loadrelocs	elf32_loadrelocs
#define	elf_symadd	elf32_symadd
#define	elf_objadd	elf32_objadd
#elif ELFSIZE == 64
#define	ELF_HDR(h)	((h).elf64)
#define	ELF_SYM(h)	((h).sym64)
#define	ldnote		ldnote64
#define	ldmap		ldmap64
#define	ldmap_obj	ldmap64_obj
#define	ldload		ldload64
#define	ldloadasect	ldloadasect64
#define	elf_ld_chkhdr	elf64_ld_chkhdr
#define	elf_fix_header	elf64_fix_header
#define	elf_fix_note	elf64_fix_note
#define	elf_fix_phdrs	elf64_fix_phdrs
#define	elf_fix_shdrs	elf64_fix_shdrs
#define	elf_fix_rel	elf64_fix_rel
#define	elf_fix_rela	elf64_fix_rela
#define	elf2nlist	elf64_2nlist
#define	elf_load_shdrs	elf64_load_shdrs
#define	elf_fix_shdrs	elf64_fix_shdrs
#define	elf_symload	elf64_symload
#define	elf_loadrelocs	elf64_loadrelocs
#define	elf_symadd	elf64_symadd
#define	elf_objadd	elf64_objadd
#else
#error "Unsupported ELF class"
#endif

struct {
	Elf_Note en;
	char name[16];
	int desc;
} ldnote = {
	{ 16, 4, 1 },
	LD_NOTE,
	0
};

/*
 * map all the objects into the loading order;
 * since that is done -- print out the map and the xref table;
 * NB maybe this can be moved to the ld.c
 */
const struct ldorder *
ldmap(const struct ldarch *lda)
{
	TAILQ_HEAD(, ldorder) headorder;
	const struct ldorder *order;
	struct ldorder *neworder, *ssorder;
	struct symlist *sym;
	uint64_t point;
	int i;

	sysobj.ol_sections[0].os_sect = &((Elf_Shdr *)sysobj.ol_sects)[0];
	sysobj.ol_sections[1].os_sect = &((Elf_Shdr *)sysobj.ol_sects)[1];
				 
	point = 0;
	neworder = NULL;
	TAILQ_INIT(&headorder);
	order = lda->la_order;
	for (i = 0, order = lda->la_order;
	    order->ldo_order != ldo_kaput; order++, i++) {

		if (strip && (order->ldo_flags & LD_DEBUG))
			continue;

		if (magic == OMAGIC && (order->ldo_flags & LD_NOOMAGIC))
			continue;
		if (magic == NMAGIC && (order->ldo_flags & LD_NONMAGIC))
			continue;
		if (magic == ZMAGIC && (order->ldo_flags & LD_NOZMAGIC))
			continue;

		switch (order->ldo_order) {
		case ldo_option:
			/* TODO parse options */
			break;

		case ldo_expr:
			/* TODO calculate new expression */
			break;

		case ldo_section:
			neworder = order_clone(lda, order);
			neworder->ldo_start =
			neworder->ldo_addr = point;
			/* roll thru all objs and map the ordered sections */
			if (obj_foreach(ldmap_obj, neworder)) {
				free(neworder);
				return NULL;
			}
			TAILQ_INSERT_TAIL(&headorder, neworder, ldo_entry);
			point = neworder->ldo_addr;
			break;

		case ldo_symbol:
			/* entry has been already handled in ldinit() */
			if (order->ldo_flags & LD_ENTRY)
				break;
			if ((sym = sym_isdefined(order->ldo_name)))
				warnx("%s: already defined in %s",
				    order->ldo_name, sym->sl_obj->ol_name);
			else {
				Elf_Sym esym;

				esym.st_name = 0;
				esym.st_value = point;
				esym.st_size = 0;
				esym.st_info =
				    ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
				esym.st_other = 0;
				esym.st_shndx = SHN_ABS;
				if ((sym = sym_isundef(order->ldo_name)))
					sym_define(sym, &sysobj, &esym);
				else
					sym = sym_add(order->ldo_name,
					    &sysobj, &esym);
			}
			break;

		case ldo_interp:
			neworder = order_clone(lda, order);
			neworder->ldo_wurst = strdup(LD_INTERP);
			neworder->ldo_wsize = sizeof LD_INTERP;
			neworder->ldo_start =
			neworder->ldo_addr = point;
			neworder->ldo_addr += ALIGN(neworder->ldo_wsize);
			TAILQ_INSERT_TAIL(&headorder, neworder, ldo_entry);
			point = neworder->ldo_addr;
			break;

		case ldo_note:
			neworder = order_clone(lda, order);
			neworder->ldo_wurst = &ldnote;
			neworder->ldo_wsize = sizeof ldnote;
			TAILQ_INSERT_TAIL(&headorder, neworder, ldo_entry);
			break;

		case ldo_shstr:
			ssorder = neworder = order_clone(lda, order);
			/* the wurst will be filled w/ strings */
			TAILQ_INSERT_TAIL(&headorder, neworder, ldo_entry);
			break;

		case ldo_kaput:
			err(1, "kaputziener");
		}
	}

	if (ssorder) {
/* TODO following neworder generate .shstr and the sh_name's */
	}

	if (printmap) {
		TAILQ_FOREACH(order, &headorder, ldo_entry) {
			struct section *sect;

			printf("%-16s0x%08llx\t0x%llx\n", order->ldo_name,
			    order->ldo_start,
			    order->ldo_addr - order->ldo_start);

			TAILQ_FOREACH(sect, &order->ldo_seclst, os_entry) {
				struct symlist *sym;
				struct objlist *ol;
				int head = 0;

				ol = sect->os_obj;
				TAILQ_FOREACH(sym, &ol->ol_syms, sl_entry) {
					Elf_Sym *esym;

					esym = &ELF_SYM(sym->sl_elfsym);
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

	/* save the entry point address */
	ELF_HDR(sysobj.ol_hdr).e_entry = ELF_SYM(sentry->sl_elfsym).st_value;

	return TAILQ_FIRST(&headorder);
}

/*
 * map an object assigning addresses the to defined symbols;

TODO
 * aditionally shuffle the sections of the same kind to provide
 * randomness to the symbol offsets; one can argue that debugging
 * would be hell due to heisenbugs elluding their beholder but
 * contrary to that by defixing their appearance we will pin on
 * their vectors way more easily.
 */
int
ldmap_obj(struct objlist *ol, void *v)
{
	uint64_t align;
	struct ldorder *neworder = v;
	Elf_Shdr *shdr = ol->ol_sects;
	struct section *sect = ol->ol_sections;
	struct symlist *sym;
	char *sname;
	int i, n;

	n = ELF_HDR(ol->ol_hdr).e_shnum;
	for (i = 0; i < n; i++, sect++, shdr++) {
		sname = ol->ol_snames + shdr->sh_name;
		if (strcmp(sname, neworder->ldo_name))
			continue;

		shdr->sh_addr = neworder->ldo_addr;
		neworder->ldo_addr += shdr->sh_size;
/* TODO align addr to shdr->sh_addralign */
		align = shdr->sh_addralign;
		neworder->ldo_addr += align - 1;
		neworder->ldo_addr &= ~(align - 1);

		/* assign addrs to all symbols in this section */
		TAILQ_FOREACH(sym, &ol->ol_syms, sl_entry) {
			Elf_Sym *esym = &ELF_SYM(sym->sl_elfsym);

			if (esym->st_shndx != i)
				continue;

			esym->st_value += shdr->sh_addr;
		}

		TAILQ_INSERT_TAIL(&neworder->ldo_seclst, sect, os_entry);
		break;
	}

	return 0;
}

int
ldload(const char *name, const struct ldorder *order)
{
	char obuf[ELF_OBUFSZ];
	off_t off, symoff;
	Elf_Ehdr *eh;
	Elf_Phdr *phdr;
	Elf_Shdr *shdr;
	FILE *fp;
	const struct ldorder *ord;
	struct section *os;
	int nsect, nphdr, shstridx;

	if (!order || errors)
		return 1;

	eh = &ELF_HDR(sysobj.ol_hdr);

	/*
	 * stroll through the order counting {e,p,s}hdrs;
	 * while here also assign file offsets for the sections;
	 */
	for (nsect = 1, nphdr = 0, ord = order; ord != TAILQ_END(NULL);
	    ord = TAILQ_NEXT(ord, ldo_entry)) {

		nsect++;

		/* XXX for now */
		nphdr++;
	}

	if (!nsect || !nphdr)
		errx(1, "output headers botch");

	/* if symtab not stripped then add two more for sym+strtabs */
	if (strip < 2)
		nsect += 2;

	if (!(phdr = calloc(nphdr, sizeof *phdr)))
		err(1, "calloc");

	if (!(shdr = calloc(nsect, sizeof *shdr)))
		err(1, "calloc");

	eh->e_ident[EI_MAG0] = ELFMAG0;
	eh->e_ident[EI_MAG1] = ELFMAG1;
	eh->e_ident[EI_MAG2] = ELFMAG2;
	eh->e_ident[EI_MAG3] = ELFMAG3;
	eh->e_ident[EI_CLASS] = elfclass;
	eh->e_ident[EI_DATA] = endian;
	eh->e_ident[EI_VERSION] = EV_CURRENT;
	eh->e_ident[EI_OSABI] = ELFOSABI_SYSV;
	eh->e_ident[EI_ABIVERSION] = 0;
	eh->e_type = relocatable? ET_REL : Bflag == 2? ET_DYN : ET_EXEC;
	eh->e_machine = machine;
	eh->e_version = EV_CURRENT;

	if (!(fp = fopen(name, "w+")))
		err(1, "fopen: %s", name);
	setvbuf(fp, obuf, _IOFBF, BUFSIZ);

	off = sizeof *eh + nphdr * sizeof *phdr;
	if (fseeko(fp, off, SEEK_SET))
		err(1, "fseeko: %s", name);

	/* dump out sections */
	for (symoff = 0, ord = order; ord != TAILQ_END(NULL);
	    ord = TAILQ_NEXT(ord, ldo_entry)) {
		if (ord->ldo_flags & LD_CONTAINS) {
			if (fwrite(ord->ldo_wurst, ord->ldo_wsize, 1, fp) != 1)
				err(1, "fwrite: %s", name);
			off = ALIGN(ord->ldo_wsize) - ord->ldo_wsize;
			if (off && fseeko(fp, off, SEEK_CUR))
				err(1, "fseeko: %s", name);
			/*
			 * section headers go before first debugging section;
			 * this also gives us a little offset for the followings
			 */
			if (ord->ldo_order == ldo_shstr) {
				symoff = nsect * sizeof *shdr;
/* TODO write out the section headers */
				if (fseeko(fp, symoff, SEEK_CUR))
					err(1, "fseeko: %s", name);
			}
			continue;
		}

		/*
		 * scan thru the sections alike and dump 'em all out
		 * into the a.out;
		 * either dump per object or per order depending
		 * what is more of we have moderated by the open
		 * file limit.
		 */
		TAILQ_FOREACH(os, &ord->ldo_seclst, os_entry) {
			struct section *osp, *esp;
			FILE *sfp;

			if (!(sfp = fopen(os->os_obj->ol_path, "r")))
				err(1, "fopen: %s", os->os_obj->ol_path);

			/*
			 * here we cheat and try to do less fopen()s
			 * than fseeko()s by dumping all sections from
			 * one object with one fopen!
			 */
			for (osp = os->os_obj->ol_sections,
			     esp = osp +  os->os_obj->ol_nsect;
			     osp < esp; osp++)
				if (!(osp->os_flags & SECTION_LOADED) &&
				    ldloadasect(sfp, fp, name, ord, osp))
					return -1;
				else
					osp->os_flags |= SECTION_LOADED;

			fclose(sfp);
		}
	}

	off = ftell(fp);
	elf_fix_shdrs(eh, shdr);
	if (fwrite(shdr, sizeof *shdr, nsect, fp) != nsect)
		err(1, "fwrite: %s", name);

/* TODO write out symtab */

	if (fseek(fp, sizeof *eh, SEEK_SET))
		err(1, "fseek: %s", name);

	elf_fix_phdrs(eh, phdr);
	if (fwrite(phdr, sizeof *phdr, nphdr, fp) != nphdr)
		err(1, "fwrite: %s", name);

	rewind(fp);
	eh->e_phoff = sizeof *eh;
	eh->e_shoff = off;
	eh->e_flags = 0;
	eh->e_ehsize = sizeof *eh;
	eh->e_phentsize = sizeof *phdr;
	eh->e_phnum = nphdr;
	eh->e_shentsize = sizeof *shdr;
	eh->e_shnum = nsect;
	eh->e_shstrndx = shstridx;

	elf_fix_header(eh);
	if (fwrite(eh, sizeof *eh, 1, fp) != 1)
		err(1, "fwrite: %s", name);

	fclose(fp);

	/* rejoice */
	return 0;
}

/*
 * load one section from one object fixing relocs;
 * luckily we might fit all data in one buffer and loops are small (;
 */
int
ldloadasect(FILE *fp, FILE *ofp, const char *name, const struct ldorder *ord,
    struct section *os)
{
	char sbuf[ELF_SBUFSZ];
	off_t off, sl;
	Elf_Shdr *shdr = os->os_sect;
	int len, bof;

	if (fseeko(fp, shdr->sh_offset, SEEK_SET) < 0)
		err(1, "fseeko: %s", os->os_obj->ol_name);

	for (sl = shdr->sh_size, off = 0, bof = len = 0;
	    off < sl; off += len) {
		len = MIN(sl - off, sizeof sbuf - bof);
		if (fread(sbuf + bof, len, 1, fp) != 1)
			err(1, "fread: %s", os->os_obj->ol_name);
		bof = ord->ldo_arch->la_fix(off, os, sbuf, len);
		if (bof < 0)
			return -1;
		else {
			if (fwrite(sbuf, len - bof, 1, ofp) != 1)
				err(1, "fwrite: %s", name);
			if (bof) {
				len -= bof;
				memcpy(sbuf, sbuf + len, bof);
			}
		}
	}

	return (0);
}

/*
 * load the relocations for the section;
 * prepare for loading the symbols and sort the relocs
 * by the symbol index;
 * alternatively we can load relocs after symbols
 * it all depends what is less to scan through.
 */
int
elf_loadrelocs(struct objlist *ol, struct section *s, Elf_Shdr *shdr,
    FILE *fp, off_t foff)
{
	Elf_Ehdr *eh = &ELF_HDR(ol->ol_hdr);
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
		for (i = 0; i < n; i++, r++) {
			if (fread(&rel, sizeof rel, 1, fp) != 1)
				err(1, "fread: %s", ol->ol_path);

			elf_fix_rel(eh, &rel);
			r->rl_sym = NULL;
			r->rl_addend = 0;
			r->rl_addr = rel.r_offset;
			r->rl_symidx = ELF_R_SYM(rel.r_info);
			r->rl_type = ELF_R_TYPE(rel.r_info);
		}
	} else {
		for (i = 0; i < n; i++, r++) {
			if (fread(&rela, sizeof rela, 1, fp) != 1)
				err(1, "fread: %s", ol->ol_path);

			r->rl_sym = NULL;
			r->rl_addr = rela.r_offset;
			r->rl_addend = rela.r_addend;
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

/*
 * add another symbol from the object;
 * a hook called from elf_symload(3).
 * collect undefined symbols and resolved the defined;
 * resolve relocation reference.
 */
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

/*
 * add an object to the link editing process;
 * load needed headers and relocations where available;
 * load symbols and resolve references from relocs and other
 * objects (as undefined);
 * prepare the relocs for the final output and sort by address.
 */
int
elf_objadd(struct objlist *ol, FILE *fp, off_t foff)
{
	struct section *s, *se;
	struct elf_symtab es;
	Elf_Ehdr *eh;
	Elf_Shdr *shdr;
	int i, n;

	eh = &ELF_HDR(ol->ol_hdr);
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

/*
 * check if object is editable by linking;
 * first object on the command line defines the machine
 * and endianess whilst later ones have to match.
 */
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
		return (ldarch = ldinit()) == NULL? EINVAL : 0;
	}

	if (*mach != eh->e_machine ||
	    *class != eh->e_ident[EI_CLASS] ||
	    *endian != eh->e_ident[EI_DATA]) {
		warnx("%s: incompatible file type", path);
		return EFTYPE;
	}

	return 0;
}
