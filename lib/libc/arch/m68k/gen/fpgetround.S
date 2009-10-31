/*
 * Written by J.T. Conklin, Apr 6, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetround.S,v 1.2 2009/05/23 01:24:08 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetround)
	fmovel fpcr,d0
	bfextu d0{#26:#2},d0
	rts
