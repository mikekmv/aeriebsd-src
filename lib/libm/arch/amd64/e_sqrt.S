/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: e_sqrt.S,v 1.2 2009/05/26 23:27:27 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(sqrt)
	sqrtsd	%xmm0,%xmm0
	ret
