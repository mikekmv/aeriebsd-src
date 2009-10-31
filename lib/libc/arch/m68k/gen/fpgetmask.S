/*
 * Written by Charles M. Hannum, Apr 9, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetmask.S,v 1.2 2009/05/23 01:24:08 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetmask)
	fmovel fpcr,d0
	bfextu d0{#18:#5},d0
	rts
