/*
 * Copyright (c) 2009-2010 Michael Shalayeff
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

#ifndef _LIBELF_H_
#define _LIBELF_H_

#define	ELFLIB_STRIPX	0x01
#define	ELFLIB_STRIPD	0x02

struct nlist;
struct dwarf_nebula;
struct elf_symtab {
		/* from the caller */
	const char *name;	/* objname */
	const void *ehdr;	/* file header */

		/* if empty we we will provide */
	void *shdr;		/* sections headers */
	char *shstr;		/* sections header strings */

		/* we provide these */
	char	*stab;		/* strings table for the syms */
	size_t	stabsz;		/* strings size */
	u_long	nsyms;		/* number of symbols in the table */
};

int	elf_checkoff(const char *, FILE *, off_t, off_t);

int	elf32_fix_header(Elf32_Ehdr *eh);
int	elf32_chk_header(Elf32_Ehdr *eh);
int	elf32_fix_note(Elf32_Ehdr *, Elf32_Note *);
Elf32_Shdr*elf32_load_shdrs(const char *, FILE *, off_t, const Elf32_Ehdr *);
int	elf32_save_shdrs(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *);
int	elf32_save_phdrs(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Phdr *);
Elf32_Phdr*elf32_load_phdrs(const char *, FILE *, off_t, const Elf32_Ehdr *);
int	elf32_fix_shdrs(const Elf32_Ehdr *eh, Elf32_Shdr *shdr);
int	elf32_fix_phdrs(const Elf32_Ehdr *eh, Elf32_Phdr *phdr);
int	elf32_fix_rel(Elf32_Ehdr *, Elf32_Rel *);
int	elf32_fix_rela(Elf32_Ehdr *, Elf32_Rela *);
int	elf32_fix_sym(const Elf32_Ehdr *eh, Elf32_Sym *sym);
int	elf32_2nlist(Elf32_Sym *, const Elf32_Ehdr *, const Elf32_Shdr *,
	    const char *, struct nlist *);
int	elf32_size(const Elf32_Ehdr *, const Elf32_Shdr *,
	    u_long *, u_long *, u_long *);
char	*elf32_shstrload(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *shdr);
char	*elf32_strload(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *shdr, const char *, const char *, size_t *);
int	elf32_symload(struct elf_symtab *, FILE *, off_t,
	    int (*func)(struct elf_symtab *, int, void *, void *), void *arg);

int	elf64_fix_header(Elf64_Ehdr *eh);
int	elf64_chk_header(Elf64_Ehdr *eh);
int	elf64_fix_note(Elf64_Ehdr *, Elf64_Note *);
Elf64_Shdr*elf64_load_shdrs(const char *, FILE *, off_t, const Elf64_Ehdr *);
int	elf64_save_shdrs(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *);
int	elf64_save_phdrs(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Phdr *);
Elf64_Phdr*elf64_load_phdrs(const char *, FILE *, off_t, const Elf64_Ehdr *);
int	elf64_fix_shdrs(const Elf64_Ehdr *eh, Elf64_Shdr *shdr);
int	elf64_fix_phdrs(const Elf64_Ehdr *eh, Elf64_Phdr *phdr);
int	elf64_fix_sym(const Elf64_Ehdr *eh, Elf64_Sym *sym);
int	elf64_fix_rel(Elf64_Ehdr *, Elf64_Rel *);
int	elf64_fix_rela(Elf64_Ehdr *, Elf64_Rela *);
int	elf64_2nlist(Elf64_Sym *, const Elf64_Ehdr *, const Elf64_Shdr *,
	    const char *, struct nlist *);
int	elf64_size(const Elf64_Ehdr *, const Elf64_Shdr *,
	    u_long *, u_long *, u_long *);
char	*elf64_shstrload(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *shdr);
char	*elf64_strload(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *shdr, const char *, const char *, size_t *);
int	elf64_symload(struct elf_symtab *, FILE *, off_t,
	    int (*func)(struct elf_symtab *, int, void *, void *), void *arg);

struct dwarf_nebula *elf_dwarfnebula(unsigned char, const char *, FILE *);
int	dwarf_addr2line(struct dwarf_nebula *, const char *, FILE *,
	    long long, char **, char **, int *);

#endif /* _LIBELF_H_ */
