/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: e_atan2.S,v 1.2 2009/05/26 23:27:28 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(atan2)
	fldl	 4(%esp)
	fldl	12(%esp)
	fpatan
	ret
