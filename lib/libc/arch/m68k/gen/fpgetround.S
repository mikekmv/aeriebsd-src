/*
 * Written by J.T. Conklin, Apr 6, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetround.S,v 1.1.1.1 2008/08/26 14:38:20 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetround)
	fmovel fpcr,d0
	bfextu d0{#26:#2},d0
	rts
