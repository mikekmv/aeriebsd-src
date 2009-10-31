/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: e_scalb.S,v 1.2 2009/05/26 23:27:28 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(scalb)
	fldl	12(%esp)
	fldl	4(%esp)
	fscale
	fstp	%st(1)			/* bug fix for fp stack overflow */
	ret
