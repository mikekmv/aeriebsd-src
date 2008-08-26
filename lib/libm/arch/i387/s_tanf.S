/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD$"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

/* A float's domain isn't large enough to require argument reduction. */
ENTRY(tanf)
	flds	4(%esp)
	fptan
	fstp	%st(0)
	ret
