/*
 * Copyright (c) 2004-2009 Michael Shalayeff
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LIBELF_H_
#define _LIBELF_H_

#define	ELFLIB_STRIPX	0x01
#define	ELFLIB_STRIPD	0x02

extern char *stab;

int	elf32_fix_header(Elf32_Ehdr *eh);
Elf32_Shdr*elf32_load_shdrs(const char *, FILE *, off_t, const Elf32_Ehdr *);
int	elf32_save_shdrs(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *);
Elf32_Phdr*elf32_load_phdrs(const char *, FILE *, off_t, const Elf32_Ehdr *);
int	elf32_fix_shdrs(const Elf32_Ehdr *eh, Elf32_Shdr *shdr);
int	elf32_fix_phdrs(const Elf32_Ehdr *eh, Elf32_Phdr *phdr);
int	elf32_fix_sym(const Elf32_Ehdr *eh, Elf32_Sym *sym);
int	elf32_size(const Elf32_Ehdr *, const Elf32_Shdr *,
	    u_long *, u_long *, u_long *);
char	*elf32_shstrload(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *shdr);
char	*elf32_strload(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *shdr, const char *, const char *, size_t *);
int	elf32_symloadx(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *, char *, struct nlist **, struct nlist ***,
	    size_t *, int *, const char *, const char *);
int	elf32_symload(const char *, FILE *, off_t, const Elf32_Ehdr *,
	    const Elf32_Shdr *, struct nlist **, struct nlist ***,
	    size_t *, int *);
int	elf32_strip(const char *, FILE *, const Elf32_Ehdr *,
	    struct stat *, off_t *);
int	elf32_symseed(const char *, FILE *, const Elf32_Ehdr *,
	    struct stat *, off_t *, int);

int	elf64_fix_header(Elf64_Ehdr *eh);
Elf64_Shdr*elf64_load_shdrs(const char *, FILE *, off_t, const Elf64_Ehdr *);
int	elf64_save_shdrs(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *);
Elf64_Phdr*elf64_load_phdrs(const char *, FILE *, off_t, const Elf64_Ehdr *);
int	elf64_fix_shdrs(const Elf64_Ehdr *eh, Elf64_Shdr *shdr);
int	elf64_fix_phdrs(const Elf64_Ehdr *eh, Elf64_Phdr *phdr);
int	elf64_fix_sym(const Elf64_Ehdr *eh, Elf64_Sym *sym);
int	elf64_size(const Elf64_Ehdr *, const Elf64_Shdr *,
	    u_long *, u_long *, u_long *);
char	*elf64_shstrload(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *shdr);
char	*elf64_strload(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *shdr, const char *, const char *, size_t *);
int	elf64_symloadx(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *, char *, struct nlist **, struct nlist ***,
	    size_t *, int *, const char *, const char *);
int	elf64_symload(const char *, FILE *, off_t, const Elf64_Ehdr *,
	    const Elf64_Shdr *, struct nlist **, struct nlist ***,
	    size_t *, int *);
int	elf64_strip(const char *, FILE *, const Elf64_Ehdr *,
	    struct stat *, off_t *);
int	elf64_symseed(const char *, FILE *, const Elf64_Ehdr *,
	    struct stat *, off_t *, int);

static inline unsigned int
elf_hash(const unsigned char *name)
{
	int hash, v;

	while (*name) {
		hash = (hash << 4) + *name++;
		if ((v = hash & 0xf0000000))
			hash ^= hash >> 24;
		hash &= ~v;
	}

	return hash;
}

#endif /* _LIBELF_H_ */
