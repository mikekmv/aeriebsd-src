/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_ilogb.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(ilogb)
	movsd	%xmm0,-8(%rsp)
	fldl	-8(%rsp)
	fxtract
	fstp	%st
	fistpl	-8(%rsp)
	movl	-8(%rsp),%eax
	ret
