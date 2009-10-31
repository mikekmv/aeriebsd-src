/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_scalbn.S,v 1.2 2009/05/26 23:27:28 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

ENTRY(scalbn)
	movsd	%xmm0,-8(%rsp)
	movl	%edi,-12(%rsp)
	fildl	-12(%rsp)
	fldl	-8(%rsp)
	fscale
	fstp	%st(1)		/* bug fix for fp stack overflow */
	fstpl	-8(%rsp)
	movsd	-8(%rsp),%xmm0
	ret

