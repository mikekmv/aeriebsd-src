/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_sinf.S,v 1.1.1.1 2008/08/26 14:38:52 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

/* A float's domain isn't large enough to require argument reduction. */
ENTRY(sinf)
	flds	4(%esp)
	fsin
	ret
