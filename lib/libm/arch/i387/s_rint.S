/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_rint.S,v 1.1.1.1 2008/08/26 14:38:52 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(rint)
	fldl	4(%esp)
	frndint
	ret
