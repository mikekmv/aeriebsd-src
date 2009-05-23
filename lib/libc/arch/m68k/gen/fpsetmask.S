/*
 * Written by Charles M. Hannum, Apr 9, 1995
 * Public Domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpsetmask.S,v 1.1.1.1 2008/08/26 14:38:20 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpsetmask)
	movel d2,sp@-
	fmovel fpcr,d1
	movel sp@(8),d2
	bfextu d1{#18:#5},d0
	bfins d2,d1{#18:#5}
	movel sp@+,d2
	fmovel d1,fpcr
	rts
