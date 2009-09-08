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

#include <sys/queue.h>
#include <sys/tree.h>

#define	ELF_RELSORT	16

struct pathlist {
	TAILQ_ENTRY(pathlist) pl_entry;
	const char *pl_path;
};

struct objlist;
struct symlist {
	SPLAY_ENTRY(symlist) sl_node;
	TAILQ_ENTRY(symlist) sl_entry;
	union {
		Elf32_Sym sym32;
		Elf64_Sym sym64;
	} sl_elfsym;
	struct objlist *sl_obj;	/* back-ptr into the object */
	const char *sl_name;
};

struct relist {
	uint64_t rl_addr, rl_addend;
	struct symlist *rl_sym;
	int rl_symidx;
	u_char rl_type;
};

struct section {
	TAILQ_ENTRY(section) os_entry;

	struct relist *os_rels;
	struct objlist *os_obj;
	void *os_sect;
	int os_no;	/* elf section number */
	int os_nrls;
};

struct objlist {
	TAILQ_ENTRY(objlist) ol_entry;
	union {
		struct exec aout;
		Elf32_Ehdr elf32;
		Elf64_Ehdr elf64;
	} ol_hdr;
	TAILQ_HEAD(, symlist) ol_syms;
	off_t ol_off;
	const char *ol_path;
	const char *ol_name;
	void *ol_sects;
	char *ol_snames;
	struct section *ol_sections;
	int ol_nsect;

	int ol_flags;
};

struct ldorder {
	enum  {
		ldo_kaput, ldo_option, ldo_expr, ldo_section, ldo_symbol
	}	ldo_order;
	const char *ldo_name;
	int ldo_type;

	u_long ldo_start;
	u_long ldo_addr;
	TAILQ_HEAD(, section) ldo_seclst;

	TAILQ_ENTRY(ldorder) ldo_entry;
};

extern int errors, printmap;
extern int machine, endian, elfclass;
extern const struct ldorder
    alpha_order[], amd64_order[], arm_order[], hppa_order[],
    i386_order[], m68k_order[], mips_order[], ppc_order[],
    sh_order[], sparc_order[], sparc64_order[], vax_order[];

int obj_foreach(int (*)(struct objlist *, void *), void *);

/* ld2.c */
int elf32_symadd(struct elf_symtab *, int, void *, void *);
int elf64_symadd(struct elf_symtab *, int, void *, void *);
int elf32_loadrelocs(struct objlist *, struct section *, Elf_Shdr *,
    FILE *, off_t);
int elf64_loadrelocs(struct objlist *, struct section *, Elf_Shdr *,
    FILE *, off_t);
int elf32_objadd(struct objlist *, FILE *, off_t);
int elf64_objadd(struct objlist *, FILE *, off_t);
const struct ldorder *ldmap32(void);
const struct ldorder *ldmap64(void);
int ldmap32_obj(struct objlist *, void *);
int ldmap64_obj(struct objlist *, void *);
int ldload32(const char *, const struct ldorder *);
int ldload64(const char *, const struct ldorder *);
int elf32_ld_chkhdr(const char *, Elf_Ehdr *, int, int *, int *, int *);
int elf64_ld_chkhdr(const char *, Elf_Ehdr *, int, int *, int *, int *);

/* syms.c */
struct symlist *sym_undef(const char *);
struct symlist *sym_isundef(const char *);
struct symlist *sym_define(struct symlist *, struct objlist *, void *);
struct symlist *sym_add(const char *, struct objlist *, void *);
struct symlist *sym_isdefined(const char *);
int sym_undcheck(void);
int rel_symcmp(const void *, const void *);
int rel_addrcmp(const void *, const void *);
int rel_fixsyms(struct objlist *, struct symlist *, int);
