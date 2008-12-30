/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD$"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(__ieee754_sqrt)
#ifdef __i386__
	fldl	4(%esp)
	fsqrt
#else
	sqrtsd	%xmm0,%xmm0
#endif
	ret