/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_scalbnf.S,v 1.1.1.1 2008/08/26 14:38:52 root Exp $"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

.global ldexpf
	ldexpf = scalbnf

ENTRY(scalbnf)
	fildl	8(%esp)
	flds	4(%esp)
	fscale
	fstp	%st(1)		/* bug fix for fp stack overflow */
	ret
