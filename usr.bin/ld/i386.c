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
static const char rcsid[] = "$ABSD$";
#endif

#include <sys/param.h>
#include <stdio.h>
#include <elf_abi.h>
#include <elfuncs.h>
#include <a.out.h>

#include "ld.h"

const struct ldorder i386_order[] = {
	{ ldo_symbol,	"_start", N_UNDF },
	{ ldo_section,	".interp", 0 },
	{ ldo_section,	".init", N_TEXT },
	{ ldo_section,	".plt", N_TEXT },
	{ ldo_section,	".text", N_TEXT },
	{ ldo_section,	".fini", N_TEXT },
	{ ldo_symbol,	"_text", N_ABS },
		/* align for a page */
	{ ldo_section,	".rodata", N_DATA },
		/* align for a page */
	{ ldo_section,	".data", N_DATA },
	{ ldo_section,	".ctors", N_DATA },
	{ ldo_section,	".dtors", N_DATA },
	{ ldo_section,	".got", N_DATA },
	{ ldo_symbol,	"_edata", N_ABS },
	{ ldo_section,	".bss", N_DATA },
	{ ldo_symbol,	"_end", N_ABS },
	/* debug sections to follow */
};
