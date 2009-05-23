/*
 * Written by Charles M. Hannum, Apr 9, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetsticky.S,v 1.1.1.1 2008/08/26 14:38:20 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetsticky)
	fmovel fpsr,d0
	bfextu d0{#24:#5},d0
	rts
