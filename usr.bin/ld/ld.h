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

#define	LD_INTERP	"/usr/libexec/ld.so"
#define	LD_NOTE		"AerieBSD"

#define	ELF_RELSORT	16
#define	ELF_OBUFSZ	0x10000
#define	ELF_SBUFSZ	0x200	/* XXX make bigger later */

/* this is used for library path and -L */
struct pathlist {
	TAILQ_ENTRY(pathlist) pl_entry;
	const char *pl_path;
};

struct objlist;

/*
 * this describes one symbol and links it to both
 * global symbol table and per-object list.
 * used for both defined and undefined symbols.
 * once symbol is defined sl_obj is filled;
 * until then sl_obj points to the first referer object;
 * it's important to keep the pointer as it is already
 * used for reference in relocs and possibly other places.
 */
struct symlist {
	SPLAY_ENTRY(symlist) sl_node;	/* global symbol table */
	TAILQ_ENTRY(symlist) sl_entry;	/* list per object */
	union {
		Elf32_Sym sym32;
		Elf64_Sym sym64;
	} sl_elfsym;
	struct objlist *sl_obj;	/* back-ptr into the object */
	const char *sl_name;
};
extern struct symlist *sentry;

/*
 * relocation description;
 * created for both rel and rela types.
 * links to the symbol once the symbol table has been loaded.
 */
struct relist {
	uint64_t rl_addr, rl_addend;	/* reloc addr and addend */
	struct symlist *rl_sym;		/* referene symbol */
	int rl_symidx;			/* symbol's index */
	u_char rl_type;			/* relocation type */
};

/*
 * a section from one object;
 * for progbits sections relocations are loaded if available;
 * array of sections later link-listed into the order for loading.
 */
struct section {
	TAILQ_ENTRY(section) os_entry;	/* list in the order */

	struct objlist *os_obj;		/* back-ref to the object */
	void *os_sect;			/* elf section descriptor */
	struct relist *os_rels;		/* array of relocations */
	struct relist *os_rp;		/* current rel pointer */
	int os_nrls;			/* number of relocations */
	int os_no;			/* elf section number */
	int os_flags;
#define	SECTION_LOADED	0x0001
};

/*
 * an object either a file or a library member;
 * contains an array of sections and needed elf headers;
 * lists all defined symbols from this object.
 */
struct objlist {
	TAILQ_ENTRY(objlist) ol_entry;	/* global list of objects */
	union {
		struct exec aout;
		Elf32_Ehdr elf32;
		Elf64_Ehdr elf64;
	} ol_hdr;
	TAILQ_HEAD(, symlist) ol_syms;	/* all defined symbols */
	off_t ol_off;			/* offset in the library */
	const char *ol_path;		/* path to the file */
	const char *ol_name;		/* path and name of the lib member */
	void *ol_sects;			/* array of section headers */
	char *ol_snames;		/* sections names */
	struct section *ol_sections;	/* array of section descriptors */
	int ol_nsect;			/* number of sections */
};

/*
 * this describes one arch;
 */
struct ldarch {
	int	la_mach;
	int	la_flags;
	const struct ldorder *la_order;
	int	(*la_fix)(off_t, struct section *, char *, int);
};
extern const struct ldarch ldarchs[];
extern const int ldnarch;
extern const struct ldarch *ldarch;

/*
 * loading order descriptor; dual-use;
 * statically defined orders are specifications for loading;
 * according to the specification a dynamis order is created
 * containing lists of sections from all objects to be loaded;
 * special ordes allow point and other expression calculation.
 */
struct ldorder {
	enum  {
		ldo_kaput, ldo_option, ldo_expr, ldo_section, ldo_symbol,
		ldo_interp, ldo_note, ldo_shstr
	}	ldo_order;
	const char *ldo_name;	/* name of the section or global */
	int ldo_type;		/* type of the section or global */
	int ldo_flags;
#define	LD_NOZMAGIC	0x0001	/* ignore for zmagic */
#define	LD_NONMAGIC	0x0002	/* ignore for nmagic */
#define	LD_NOOMAGIC	0x0004	/* ignore for omagic */
#define	LD_WXORX	0x0008	/* only for (normal) W^X executables */
#define	LD_PIE		0x0010	/* for position-independant executables */
#define	LD_DYNAMIC	0x0020	/* for dynamic executables */
#define	LD_DEBUG	0x0100	/* debugging info (strip w/ -S) */
#define	LD_CONTAINS	0x0200	/* contents is generated in ldo_wurst */
#define	LD_ENTRY	0x1000	/* entry point */

	union {
		struct exec aout;
		Elf32_Shdr elf32;
		Elf64_Shdr elf64;
	} ldo_sect;
	uint64_t ldo_start;	/* start of this segment */
	uint64_t ldo_addr;	/* current address during section mapping */
	const void *ldo_wurst;	/* contents to dump thru */
	size_t ldo_wsize;	/* wurst size */
	const struct ldarch *ldo_arch;		/* point back to the arch */
	TAILQ_HEAD(, section) ldo_seclst;	/* all sections */

	TAILQ_ENTRY(ldorder) ldo_entry;		/* list of the order */
};

extern struct objlist sysobj;
extern const char *entry_name;
extern int Bflag, errors, printmap, relocatable, strip;
extern int machine, endian, elfclass, magic;
extern const struct ldorder
    alpha_order[], amd64_order[], arm_order[], hppa_order[],
    i386_order[], m68k_order[], mips_order[], ppc_order[],
    sh_order[], sparc_order[], sparc64_order[], vax_order[];
int     i386_fix(off_t, struct section *, char *, int);

const struct ldarch *ldinit(void);
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
const struct ldorder *ldmap32(const struct ldarch *);
const struct ldorder *ldmap64(const struct ldarch *);
int ldmap32_obj(struct objlist *, void *);
int ldmap64_obj(struct objlist *, void *);
int ldload32(const char *, const struct ldorder *);
int ldload64(const char *, const struct ldorder *);
int ldloadasect32(FILE *, FILE *, const char *, const struct ldorder *, struct section *);
int ldloadasect64(FILE *, FILE *, const char *, const struct ldorder *, struct section *);
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
struct ldorder *order_clone(const struct ldarch *, const struct ldorder *);
