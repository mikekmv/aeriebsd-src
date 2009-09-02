/*
 * Copyright (c) 2003-2009 Michael Shalayeff
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
static const char rcsid[] =
    "$ABSD: elf.c,v 1.13 2009/08/14 15:34:09 mickey Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <a.out.h>
#include <elf_abi.h>
#include "elfuncs.h"

#if ELFSIZE == 32
#define	swap_addr	swap32
#define	swap_off	swap32
#define	swap_sword	swap32
#define	swap_word	swap32
#define	swap_sxword	swap32
#define	swap_xword	swap32
#define	swap_half	swap16
#define	swap_quarter	swap16
#define	elf_fix_header	elf32_fix_header
#define	elf_load_shdrs	elf32_load_shdrs
#define	elf_save_shdrs	elf32_save_shdrs
#define	elf_load_phdrs	elf32_load_phdrs
#define	elf_fix_shdrs	elf32_fix_shdrs
#define	elf_fix_phdrs	elf32_fix_phdrs
#define	elf_fix_sym	elf32_fix_sym
#define	elf_size	elf32_size
#define	elf_shstrload	elf32_shstrload
#define	elf_strload	elf32_strload
#define	elf_symloadx	elf32_symloadx
#define	elf_symload	elf32_symload
#define	elf_symseed	elf32_symseed
#define	elf_strip	elf32_strip
#define	elf2nlist	elf32_2nlist
#define	elf_shn2type	elf32_shn2type
#elif ELFSIZE == 64
#define	swap_addr	swap64
#define	swap_off	swap64
#ifdef __alpha__
#define	swap_sword	swap64
#define	swap_word	swap64
#else
#define	swap_sword	swap32
#define	swap_word	swap32
#endif
#define	swap_sxword	swap64
#define	swap_xword	swap64
#define	swap_half	swap64
#define	swap_quarter	swap16
#define	elf_fix_header	elf64_fix_header
#define	elf_load_shdrs	elf64_load_shdrs
#define	elf_save_shdrs	elf64_save_shdrs
#define	elf_load_phdrs	elf64_load_phdrs
#define	elf_fix_shdrs	elf64_fix_shdrs
#define	elf_fix_phdrs	elf64_fix_phdrs
#define	elf_fix_sym	elf64_fix_sym
#define	elf_size	elf64_size
#define	elf_shstrload	elf64_shstrload
#define	elf_strload	elf64_strload
#define	elf_symloadx	elf64_symloadx
#define	elf_symload	elf64_symload
#define	elf_symseed	elf64_symseed
#define	elf_strip	elf64_strip
#define	elf2nlist	elf64_2nlist
#define	elf_shn2type	elf64_shn2type
#else
#error "Unsupported ELF class"
#endif

int elf_symloadx(struct elf_symtab *es, FILE *fp, off_t foff,
    int (*func)(struct elf_symtab *, void *, void *), void *arg,
    const char *, const char *);

#define	ELF_SDATA	".sdata"
#define	ELF_SBSS	".sbss"
#define	ELF_PLT		".plt"

#ifndef	SHN_MIPS_ACOMMON
#define	SHN_MIPS_ACOMMON	SHN_LOPROC + 0
#endif
#ifndef	SHN_MIPS_TEXT
#define	SHN_MIPS_TEXT		SHN_LOPROC + 1
#endif
#ifndef	SHN_MIPS_DATA
#define	SHN_MIPS_DATA		SHN_LOPROC + 2
#endif
#ifndef	SHN_MIPS_SUNDEFINED
#define	SHN_MIPS_SUNDEFINED	SHN_LOPROC + 4
#endif
#ifndef	SHN_MIPS_SCOMMON
#define	SHN_MIPS_SCOMMON	SHN_LOPROC + 3
#endif

#ifndef	STT_PARISC_MILLI
#define	STT_PARISC_MILLI	STT_LOPROC + 0
#endif

int
elf_fix_header(Elf_Ehdr *eh)
{
	/* nothing to do */
	if (eh->e_ident[EI_DATA] == ELF_TARG_DATA)
		return (0);

	eh->e_type = swap16(eh->e_type);
	eh->e_machine = swap16(eh->e_machine);
	eh->e_version = swap32(eh->e_version);
	eh->e_entry = swap_addr(eh->e_entry);
	eh->e_phoff = swap_off(eh->e_phoff);
	eh->e_shoff = swap_off(eh->e_shoff);
	eh->e_flags = swap32(eh->e_flags);
	eh->e_ehsize = swap16(eh->e_ehsize);
	eh->e_phentsize = swap16(eh->e_phentsize);
	eh->e_phnum = swap16(eh->e_phnum);
	eh->e_shentsize = swap16(eh->e_shentsize);
	eh->e_shnum = swap16(eh->e_shnum);
	eh->e_shstrndx = swap16(eh->e_shstrndx);

	return (1);
}

Elf_Shdr *
elf_load_shdrs(const char *name, FILE *fp, off_t foff, const Elf_Ehdr *head)
{
	Elf_Shdr *shdr;

	if ((shdr = calloc(head->e_shnum, head->e_shentsize)) == NULL) {
		warn("%s: malloc shdr", name);
		return (NULL);
	}

	if (fseeko(fp, foff + head->e_shoff, SEEK_SET)) {
		warn("%s: fseeko", name);
		free(shdr);
		return (NULL);
	}

	if (fread(shdr, head->e_shentsize, head->e_shnum, fp) != head->e_shnum) {
		warnx("%s: premature EOF", name);
		free(shdr);
		return (NULL);
	}

	return (shdr);
}

int
elf_save_shdrs(const char *name, FILE *fp, off_t foff, const Elf_Ehdr *head,
    const Elf_Shdr *shdr)
{
	if (fseeko(fp, foff + head->e_shoff, SEEK_SET)) {
		warn("%s: fseeko", name);
		return (1);
	}

	if (fwrite(shdr, head->e_shentsize, head->e_shnum, fp) != head->e_shnum) {
		warnx("%s: premature EOF", name);
		return (1);
	}

	return (0);
}

Elf_Phdr *
elf_load_phdrs(const char *name, FILE *fp, off_t foff, const Elf_Ehdr *head)
{
	Elf_Phdr *phdr;

	if ((phdr = calloc(head->e_phnum, head->e_phentsize)) == NULL) {
		warn("%s: malloc phdr", name);
		return (NULL);
	}

	if (fseeko(fp, foff + head->e_phoff, SEEK_SET)) {
		warn("%s: fseeko", name);
		free(phdr);
		return (NULL);
	}

	if (fread(phdr, head->e_phentsize, head->e_phnum, fp) != head->e_phnum) {
		warnx("%s: premature EOF", name);
		free(phdr);
		return (NULL);
	}

	return (phdr);
}

int
elf_fix_shdrs(const Elf_Ehdr *eh, Elf_Shdr *shdr)
{
	int i;

	/* nothing to do */
	if (eh->e_ident[EI_DATA] == ELF_TARG_DATA)
		return (0);

	for (i = eh->e_shnum; i--; shdr++) {
		shdr->sh_name = swap32(shdr->sh_name);
		shdr->sh_type = swap32(shdr->sh_type);
		shdr->sh_flags = swap_xword(shdr->sh_flags);
		shdr->sh_addr = swap_addr(shdr->sh_addr);
		shdr->sh_offset = swap_off(shdr->sh_offset);
		shdr->sh_size = swap_xword(shdr->sh_size);
		shdr->sh_link = swap32(shdr->sh_link);
		shdr->sh_info = swap32(shdr->sh_info);
		shdr->sh_addralign = swap_xword(shdr->sh_addralign);
		shdr->sh_entsize = swap_xword(shdr->sh_entsize);
	}

	return (1);
}

int
elf_fix_phdrs(const Elf_Ehdr *eh, Elf_Phdr *phdr)
{
	int i;

	/* nothing to do */
	if (eh->e_ident[EI_DATA] == ELF_TARG_DATA)
		return (0);

	for (i = eh->e_phnum; i--; phdr++) {
		phdr->p_type = swap32(phdr->p_type);
		phdr->p_flags = swap32(phdr->p_flags);
		phdr->p_offset = swap_off(phdr->p_offset);
		phdr->p_vaddr = swap_addr(phdr->p_vaddr);
		phdr->p_paddr = swap_addr(phdr->p_paddr);
		phdr->p_filesz = swap_xword(phdr->p_filesz);
		phdr->p_memsz = swap_xword(phdr->p_memsz);
		phdr->p_align = swap_xword(phdr->p_align);
	}

	return (1);
}

int
elf_fix_sym(const Elf_Ehdr *eh, Elf_Sym *sym)
{
	/* nothing to do */
	if (eh->e_ident[EI_DATA] == ELF_TARG_DATA)
		return (0);

	sym->st_name = swap32(sym->st_name);
	sym->st_shndx = swap16(sym->st_shndx);
	sym->st_value = swap_addr(sym->st_value);
	sym->st_size = swap_xword(sym->st_size);

	return (1);
}

int
elf_shn2type(const Elf_Ehdr *eh, u_int shn, const char *sn)
{
	switch (shn) {
	case SHN_MIPS_SUNDEFINED:
		if (eh->e_machine == EM_MIPS)
			return (N_UNDF | N_EXT);
		break;

	case SHN_UNDEF:
		return (N_UNDF | N_EXT);

	case SHN_ABS:
		return (N_ABS);

	case SHN_MIPS_ACOMMON:
		if (eh->e_machine == EM_MIPS)
			return (N_COMM);
		break;

	case SHN_MIPS_SCOMMON:
		if (eh->e_machine == EM_MIPS)
			return (N_COMM);
		break;

	case SHN_COMMON:
		return (N_COMM);

	case SHN_MIPS_TEXT:
		if (eh->e_machine == EM_MIPS)
			return (N_TEXT);
		break;

	case SHN_MIPS_DATA:
		if (eh->e_machine == EM_MIPS)
			return (N_DATA);
		break;

	default:
		/* beyond 8 a table-driven binsearch shall be used */
		if (sn == NULL)
			return (-1);
		else if (!strcmp(sn, ELF_TEXT))
			return (N_TEXT);
		else if (!strcmp(sn, ELF_RODATA))
			return (N_SIZE);
		else if (!strcmp(sn, ELF_DATA))
			return (N_DATA);
		else if (!strcmp(sn, ELF_SDATA))
			return (N_DATA);
		else if (!strcmp(sn, ELF_BSS))
			return (N_BSS);
		else if (!strcmp(sn, ELF_SBSS))
			return (N_BSS);
		else if (!strncmp(sn, ELF_GOT, sizeof(ELF_GOT) - 1))
			return (N_DATA);
		else if (!strncmp(sn, ELF_PLT, sizeof(ELF_PLT) - 1))
			return (N_DATA);
	}

	return (-1);
}

/*
 * Devise nlist's type from Elf_Sym.
 * XXX this task is done as well in libc and kvm_mkdb.
 */
int
elf2nlist(Elf_Sym *sym, const Elf_Ehdr *eh, const Elf_Shdr *shdr,
    const char *shstr, struct nlist *np)
{
	u_char stt;
	const char *sn;
	int type;

	if (sym->st_shndx < eh->e_shnum)
		sn = shstr + shdr[sym->st_shndx].sh_name;
	else
		sn = NULL;

	switch (stt = ELF_ST_TYPE(sym->st_info)) {
	case STT_NOTYPE:
	case STT_OBJECT:
		type = elf_shn2type(eh, sym->st_shndx, sn);
		if (type < 0) {
			if (sn == NULL)
				np->n_other = '?';
			else
				np->n_type = stt == STT_NOTYPE? N_COMM : N_DATA;
		} else {
			/* a hack for .rodata check (; */
			if (type == N_SIZE) {
				np->n_type = N_DATA;
				np->n_other = 'r';
			} else
				np->n_type = type;
		}
		break;

	case STT_FUNC:
		type = elf_shn2type(eh, sym->st_shndx, NULL);
		np->n_type = type < 0? N_TEXT : type;
		if (ELF_ST_BIND(sym->st_info) == STB_WEAK) {
			np->n_type = N_INDR;
			np->n_other = 'W';
		} else if (sn != NULL && *sn != 0 &&
		    strcmp(sn, ELF_INIT) &&
		    strcmp(sn, ELF_TEXT) &&
		    strcmp(sn, ELF_FINI))	/* XXX GNU compat */
			np->n_other = '?';
		break;

	case STT_SECTION:
		type = elf_shn2type(eh, sym->st_shndx, NULL);
		if (type < 0)
			np->n_other = '?';
		else
			np->n_type = type;
		break;

	case STT_FILE:
		np->n_type = N_FN | N_EXT;
		break;

	case STT_PARISC_MILLI:
		if (eh->e_machine == EM_PARISC)
			np->n_type = N_TEXT;
		else
			np->n_other = '?';
		break;

	default:
		np->n_other = '?';
		break;
	}
	if (np->n_type != N_UNDF && ELF_ST_BIND(sym->st_info) != STB_LOCAL) {
		np->n_type |= N_EXT;
		if (np->n_other)
			np->n_other = toupper(np->n_other);
	}

	np->n_value = sym->st_value;
	np->n_un.n_strx = sym->st_name;
	return (0);
}

int
elf_size(const Elf_Ehdr *head, const Elf_Shdr *shdr,
    u_long *ptext, u_long *pdata, u_long *pbss)
{
	int i;

	*ptext = *pdata = *pbss = 0;

	for (i = 0; i < head->e_shnum; i++) {
		if (!(shdr[i].sh_flags & SHF_ALLOC))
			;
		else if (shdr[i].sh_flags & SHF_EXECINSTR ||
		    !(shdr[i].sh_flags & SHF_WRITE))
			*ptext += shdr[i].sh_size;
		else if (shdr[i].sh_type == SHT_NOBITS)
			*pbss += shdr[i].sh_size;
		else
			*pdata += shdr[i].sh_size;
	}

	return (0);
}

char *
elf_strload(const char *name, FILE *fp, off_t foff, const Elf_Ehdr *eh,
    const Elf_Shdr *shdr, const char *shstr, const char *strtab,
    size_t *pstabsize)
{
	char *ret = NULL;
	int i;

	for (i = 0; i < eh->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_STRTAB &&
		    !strcmp(shstr + shdr[i].sh_name, strtab)) {
			*pstabsize = shdr[i].sh_size;
			if (*pstabsize > SIZE_T_MAX) {
				warnx("%s: corrupt file", name);
				return (NULL);
			}

			if ((ret = malloc(*pstabsize)) == NULL) {
				warn("malloc");
				break;
			} else if (pread(fileno(fp), ret, *pstabsize,
			    foff + shdr[i].sh_offset) != *pstabsize) {
				warn("pread: %s", name);
				free(ret);
				ret = NULL;
				break;
			}
		}
	}

	return ret;
}

int
elf_symloadx(struct elf_symtab *es, FILE *fp, off_t foff,
    int (*func)(struct elf_symtab *, void *, void *), void *arg,
    const char *strtab, const char *symtab)
{
	const Elf_Ehdr *eh = es->ehdr;
	const Elf_Shdr *shdr = es->shdr;
	long symsize;
	Elf_Sym sbuf;
	int i;

	if (!(es->stab = elf_strload(es->name, fp, foff, eh, shdr, es->shstr,
	    strtab, &es->stabsz)))
		return (1);

	for (i = 0; i < eh->e_shnum; i++) {
		if (!strcmp(es->shstr + shdr[i].sh_name, symtab)) {
			symsize = shdr[i].sh_size;
			if (fseeko(fp, foff + shdr[i].sh_offset, SEEK_SET)) {
				warn("%s: fseeko", es->name);
				free(es->stab);
				return (1);
			}

			es->nsyms = symsize / sizeof sbuf;
			for (; symsize > 0; symsize -= sizeof sbuf) {
				if (fread(&sbuf, sizeof sbuf, 1, fp) != 1) {
					warn("%s: read symbol", es->name);
					free(es->stab);
					return (1);
				}

				elf_fix_sym(eh, &sbuf);
				if (!sbuf.st_name || sbuf.st_name > es->stabsz)
					continue;

				if ((*func)(es, &sbuf, arg)) {
					free(es->stab);
					return 1;
				}
			}
		}
	}
	return (0);
}

char *
elf_shstrload(const char *name, FILE *fp, off_t foff, const Elf_Ehdr *eh,
    const Elf_Shdr *shdr)
{
	long shstrsize;
	char *shstr;

	if (!eh->e_shstrndx || eh->e_shstrndx >= eh->e_shnum) {
		warnx("%s: invalid header", name);
		return NULL;
	}

	shstrsize = shdr[eh->e_shstrndx].sh_size;
	if (shstrsize == 0) {
		warnx("%s: no section name list", name);
		return (NULL);
	}

	if ((shstr = malloc(shstrsize)) == NULL) {
		warn("%s: malloc shsrt", name);
		return (NULL);
	}

	if (fseeko(fp, foff + shdr[eh->e_shstrndx].sh_offset, SEEK_SET)) {
		warn("%s: fseeko", name);
		free(shstr);
		return (NULL);
	}

	if (fread(shstr, shstrsize, 1, fp) != 1) {
		warnx("%s: premature EOF", name);
		free(shstr);
		return (NULL);
	}

	return shstr;
}

int
elf_symload(struct elf_symtab *es, FILE *fp, off_t foff,
    int (*func)(struct elf_symtab *, void *, void *), void *arg)
{
	int rv;

	if (!es->shdr &&
	    !(es->shdr = elf_load_shdrs(es->name, fp, foff, es->ehdr)))
		return 1;

	elf_fix_shdrs(es->ehdr, es->shdr);
	if (!es->shstr && !(es->shstr = elf_shstrload(es->name, fp, foff,
	    es->ehdr, es->shdr)))
		return 1;

	es->stab = NULL;
	if (elf_symloadx(es, fp, foff, func, arg, ELF_STRTAB, ELF_SYMTAB)) {
		free(es->stab);
		es->stab = NULL;
		if ((rv = elf_symloadx(es, fp, foff, func, arg,
		    ELF_DYNSTR, ELF_DYNSYM))) {
			free(es->stab);
			return rv;
		}
	}

	return (0);
}

int
elf_strip(const char *name, FILE *fp, const Elf_Ehdr *eh,
    struct stat *sb, off_t *nszp)
{
	Elf_Shdr *shdr, *sp;
	char *shstr;
	size_t sz;
	off_t off;
	int i, sn, rv = 0;

	if (!(shdr = elf_load_shdrs(name, fp, 0, eh)))
		return 1;

	if (!(shstr = elf_shstrload(name, fp, 0, eh, shdr))) {
		free(shdr);
		return 1;
	}

	sn = 0; sz = 0; off = 0;
	for (i = eh->e_shnum, sp = shdr; i--; sp++) {
		if (off && sp->sh_type == SHT_STRTAB) {
			if (sp->sh_offset > sb->st_size ||
			    sp->sh_offset < off + sz) {
				warnx("%s: corrupt section header", name);
				rv = 1;
				break;
			}
			sz = sp->sh_offset + sp->sh_size - off;
			continue;
		}

		if (sp->sh_type != SHT_SYMTAB ||
		    strcmp(shstr + sp->sh_name, ELF_SYMTAB)) {
			if (off) {
				warnx("%s: symtab in the middle", name);
				rv = 1;
				break;
			}
			continue;
		}

		sn = sp - shdr;
		if (sp->sh_offset > sb->st_size) {
			warnx("%s: corrupt section header", name);
			rv = 1;
			break;
		}
		off = sp->sh_offset;
		sz = sp->sh_size;
	}

	/* do the dew */
	if (!rv && sn) {
		shdr[sn].sh_type = SHT_NULL;
		shdr[sn+1].sh_type = SHT_NULL;
		elf_fix_shdrs(eh, shdr);
		if (elf_save_shdrs(name, fp, 0, eh, shdr))
			rv = 1;
		else
			*nszp = off;
	}

	free(shstr);
	free(shdr);
	return (rv);
}

int
elf_symseed(const char *name, FILE *fp, const Elf_Ehdr *eh,
    struct stat *sb, off_t *nszp, int strip)
{

	return (0);
}
