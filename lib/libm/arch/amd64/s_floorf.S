/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_floorf.S,v 1.2 2009/05/26 23:27:27 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(floorf)
	movss	%xmm0, -4(%rsp)
	fstcw	-8(%rsp)
	movw	-8(%rsp),%dx
	orw	$0x0400,%dx
	andw	$0xf7ff,%dx  
	movw	%dx,-12(%rsp)
	fldcw	-12(%rsp)
	flds	-4(%rsp)
	frndint
	fldcw	-8(%rsp)
	fstps	-4(%rsp)
	movss	-4(%rsp),%xmm0
	ret
