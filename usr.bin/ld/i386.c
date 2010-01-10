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
static const char rcsid[] = "$ABSD: i386.c,v 1.2 2009/10/23 21:28:03 mickey Exp $";
#endif

#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <elf_abi.h>
#include <dwarf.h>
#include <elfuncs.h>
#include <a.out.h>
#include <err.h>

#include <i386/reloc.h>

#include "ld.h"

#define	ELF_NOTE	".note.aeriebsd.ident"
#define	ELF_STAB	".stab"
#define	ELF_STABSTR	".stabstr"
#define	ELF_STAB_EXCL	".stab.excl"
#define	ELF_STAB_EXCLS	".stab.exclstr"
#define	ELF_STAB_INDEX	".stab.index"
#define	ELF_STAB_IDXSTR	".stab.indexstr"
#define	ELF_STAB_COMM	".comment"

const struct ldorder i386_order[] = {
	{ ldo_symbol,	"_start", N_UNDF, 0, LD_ENTRY },
	{ ldo_interp,	ELF_INTERP, SHT_PROGBITS, SHF_ALLOC,
	  LD_CONTAINS | LD_DYNAMIC },
	{ ldo_note,	ELF_NOTE, SHT_NOTE, SHF_ALLOC, LD_CONTAINS },
	{ ldo_section,	ELF_INIT, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR },
	{ ldo_section,	ELF_PLT, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR,
	  LD_DYNAMIC },
	{ ldo_section,	ELF_TEXT, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR },
	{ ldo_section,	ELF_FINI, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"etext", N_ABS },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_section,	ELF_RODATA, SHT_PROGBITS, SHF_ALLOC },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"__data_start", N_ABS },
	{ ldo_section,	ELF_SDATA, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE },
	{ ldo_section,	ELF_DATA, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE },
	{ ldo_section,	ELF_CTORS, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE },
	{ ldo_section,	ELF_DTORS, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"__got_start", N_ABS },
	{ ldo_section,	ELF_GOT, SHT_PROGBITS, SHF_ALLOC, LD_DYNAMIC },
	{ ldo_symbol,	"__got_end", N_ABS },
	{ ldo_symbol,	"edata", N_ABS },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"__bss_start", N_ABS },
	{ ldo_section,	ELF_SBSS, SHT_NOBITS, SHF_ALLOC | SHF_WRITE },
	{ ldo_section,	ELF_BSS, SHT_NOBITS, SHF_ALLOC | SHF_WRITE },
	{ ldo_symbol,	"end", N_ABS },
	{ ldo_shstr,	ELF_SHSTRTAB, SHT_STRTAB, 0, LD_CONTAINS },
	  /* section headers go here */
	{ ldo_symtab,	ELF_SYMTAB, SHT_SYMTAB, 0, LD_CONTAINS | LD_SYMTAB },
	{ ldo_strtab,	ELF_STRTAB, SHT_STRTAB, 0, LD_CONTAINS | LD_SYMTAB },
	  /* stabs debugging sections */
	{ ldo_section,	ELF_STAB, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	ELF_STABSTR, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	ELF_STAB_EXCL, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	ELF_STAB_EXCLS, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	ELF_STAB_INDEX, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	ELF_STAB_IDXSTR, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	ELF_STAB_COMM, SHT_PROGBITS, 0, LD_DEBUG },
	  /* dwarf mark II debugging sections */
	{ ldo_section,	DWARF_ABBREV, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_ARANGES, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_FRAMES, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_INFO, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_LINES, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_LOC, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_MACROS, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_PUBNAMES, SHT_PROGBITS, 0, LD_DEBUG },
	{ ldo_section,	DWARF_STR, SHT_PROGBITS, 0, LD_DEBUG },
};

int
i386_fix(off_t off, struct section *os, char *sbuf, int len)
{
	struct relist *rp = os->os_rels, *erp = rp + os->os_nrls;
	Elf32_Shdr *shdr = os->os_sect;
	char *p, *ep;
	uint32_t v32;
	int reoff;

/* this evil little loop has to be optimised; for example store last rp on os */
	for (; rp < erp; rp++)
		if (off <= rp->rl_addr)
			break;

	if (rp == erp)
		return 0;

fprintf(stderr, "%llx %llx\n", off, rp->rl_addr);

	for (p = sbuf + rp->rl_addr - off, ep = sbuf + len;
	    p < ep; p += reoff, rp++) {

		if (rp >= erp)
			errx(1, "i386_fix: botch1");

		if (rp + 1 < erp)
			reoff = rp[1].rl_addr - rp[0].rl_addr;
		else
			reoff = ep - p;

		switch (rp->rl_type) {
		case RELOC_32:
			if (ep - p < 4)
				return ep - p;
if (!rp->rl_sym) continue;
			/* it may be unaligned so copy out */
			memcpy(&v32, p, sizeof v32);
			v32 += rp->rl_sym->sl_elfsym.sym32.st_value +
			    rp->rl_addend;
fprintf(stderr, "a %s %x\n", rp->rl_sym->sl_name, rp->rl_sym->sl_elfsym.sym32.st_value);
			memcpy(p, &v32, sizeof v32);
			break;

		case RELOC_PC32:
			if (ep - p < 4)
				return ep - p;
if (!rp->rl_sym) continue;
			/* it may be unaligned so copy out */
			memcpy(&v32, p, sizeof v32);
			v32 += rp->rl_sym->sl_elfsym.sym32.st_value -
			    (shdr->sh_addr + rp->rl_addr) + rp->rl_addend;
fprintf(stderr, "i %s %x %llx\n", rp->rl_sym->sl_name, rp->rl_sym->sl_elfsym.sym32.st_value, shdr->sh_addr + rp->rl_addr);
			memcpy(p, &v32, sizeof v32);
			break;

		case RELOC_GOT32:
		case RELOC_PLT32:
		case RELOC_COPY:
		case RELOC_GLOB_DAT:
		case RELOC_JUMP_SLOT:
		case RELOC_RELATIVE:
		case RELOC_GOTOFF:
		case RELOC_GOTPC:
		case RELOC_16:
		case RELOC_PC16:
		case RELOC_8:
		case RELOC_PC8:

		case RELOC_NONE:
		default:
			errx(1, "unknown reloc type %d", rp->rl_type);
		}
	}

	return 0;
}
