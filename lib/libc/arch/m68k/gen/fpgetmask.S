/*
 * Written by Charles M. Hannum, Apr 9, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.rodata
	.asciz "$ABSD$"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetmask)
	fmovel fpcr,d0
	bfextu d0{#18:#5},d0
	rts