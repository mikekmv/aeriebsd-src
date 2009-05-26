/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: e_remainder.S,v 1.1.1.1 2008/08/26 14:38:52 root Exp $"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(remainder)
	fmoved	sp@(4),fp0
	fremd	sp@(12),fp0
	fmoved	fp0,sp@-
	movel	sp@+,d0
	movel	sp@+,d1
	rts
