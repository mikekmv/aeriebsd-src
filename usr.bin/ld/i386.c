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
static const char rcsid[] = "$ABSD: i386.c,v 1.1 2009/09/04 09:34:05 mickey Exp $";
#endif

#include <sys/param.h>
#include <stdio.h>
#include <elf_abi.h>
#include <elfuncs.h>
#include <a.out.h>
#include <err.h>

#include <i386/reloc.h>

#include "ld.h"

const struct ldorder i386_order[] = {
	{ ldo_symbol,	"_start", N_UNDF, LD_ENTRY },
	{ ldo_interp,	".interp", 0, LD_CONTAINS | LD_DYNAMIC },
	{ ldo_note,	".note.aeriebsd.ident", 0, LD_CONTAINS },
	{ ldo_section,	".init", N_TEXT },
	{ ldo_section,	".plt", N_TEXT },
	{ ldo_section,	".text", N_TEXT },
	{ ldo_section,	".fini", N_TEXT },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_section,	".plt", N_TEXT },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"etext", N_ABS },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_section,	".rodata", N_DATA },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"__data_start", N_ABS },
	{ ldo_section,	".sdata", N_DATA },
	{ ldo_section,	".data", N_DATA },
	{ ldo_section,	".ctors", N_DATA },
	{ ldo_section,	".dtors", N_DATA },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"__got_start", N_ABS },
	{ ldo_section,	".got", N_DATA },
	{ ldo_symbol,	"__got_end", N_ABS },
	{ ldo_expr,	". += 0x1000", 0, LD_NOOMAGIC },
	{ ldo_symbol,	"edata", N_ABS },
	{ ldo_symbol,	"__bss_start", N_ABS },
	{ ldo_section,	".sbss", N_DATA },
	{ ldo_section,	".bss", N_DATA },
	{ ldo_symbol,	"end", N_ABS },
	{ ldo_shstr,	".shstr", 0, LD_CONTAINS },
	/* stabs debugging sections */
	{ ldo_section,	".stab", 0},
	{ ldo_section,	".stabstr", 0},
	{ ldo_section,	".stab.excl", 0},
	{ ldo_section,	".stab.exclstr", 0},
	{ ldo_section,	".stab.index", 0},
	{ ldo_section,	".stab.inexstr", 0},
	{ ldo_section,	".comment", 0},
	/* dwarf mark II debugging sections */
	{ ldo_section,	".debug_abbrev", 0 },
	{ ldo_section,	".debug_aranges", 0 },
	{ ldo_section,	".debug_frame", 0 },
	{ ldo_section,	".debug_info", 0 },
	{ ldo_section,	".debug_line", 0 },
	{ ldo_section,	".debug_loc", 0 },
	{ ldo_section,	".debug_macinfo", 0 },
	{ ldo_section,	".debug_pubnames", 0 },
	{ ldo_section,	".debug_str", 0 },
};

int
i386_fix(off_t off, struct section *os, char *sbuf, int len)
{
	struct relist *rp = os->os_rels, *erp = rp + os->os_nrls;
	char *p, *ep;
	int reoff;

	for (; rp < erp; rp++)
		if (off >= rp->rl_addr)
			break;

	if (rp == erp)
		return 0;

	for (p = sbuf + rp->rl_addr, ep = sbuf + len;
	    p < ep; p += reoff, rp++) {

		if (rp >= erp)
			errx(1, "i386_fix: botch1");

		if (rp + 1 < erp) {
			reoff = rp[1].rl_addr - rp[0].rl_addr;
			if (reoff >= ep - p)
				return ep - p;
		} else
			reoff = ep - p;

		switch (rp->rl_type) {
		case RELOC_32:
		case RELOC_PC32:
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
			break;

		case RELOC_NONE:
		default:
			errx(1, "unknown reloc type %d", rp->rl_type);
		}
	}

	return 0;
}
