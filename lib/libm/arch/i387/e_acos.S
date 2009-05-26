/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: e_acos.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

/* acos = atan (sqrt(1 - x^2) / x) */
ENTRY(acos)
	fldl	4(%esp)			/* x */
	fst	%st(1)
	fmul	%st(0)			/* x^2 */
	fld1				
	fsubp				/* 1 - x^2 */
	fsqrt				/* sqrt (1 - x^2) */
	fxch	%st(1)
	fpatan
	ret
