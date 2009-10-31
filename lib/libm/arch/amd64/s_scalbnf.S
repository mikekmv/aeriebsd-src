/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_scalbnf.S,v 1.2 2009/05/26 23:27:28 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

.global ldexpf
	ldexpf = scalbnf

ENTRY(scalbnf)
	movss	%xmm0,-8(%rsp)
	movl	%edi,-4(%rsp)
	fildl	-4(%rsp)
	flds	-8(%rsp)
	fscale
	fstp	%st(1)		/* bug fix for fp stack overflow */
	fstps	-8(%rsp)
	movss	-8(%rsp),%xmm0
	ret

