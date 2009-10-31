/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

/*
 * XXXfvdl might as well split this file.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_copysign.S,v 1.2 2009/05/26 23:27:27 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

.Lpos:
	.quad	0x8000000000000000
.Lneg:
	.quad	0x7fffffffffffffff

ENTRY(copysign)
#if 0
	/*
	 * XXXfvdl gas doesn't grok this yet.
	 */
	movq	.Lpos(%rip),%xmm2
	movq	.Lneg(%rip),%xmm3
	pand	%xmm2,%xmm1
	pand	%xmm3,%xmm0
	por	%xmm1,%xmm0
#else
	movsd	%xmm0,-8(%rsp)
	movsd	%xmm1,-16(%rsp)
	movl	-12(%rsp),%edx
	andl	$0x80000000,%edx
	movl	-4(%rsp),%eax
	andl	$0x7fffffff,%eax
	orl	%edx,%eax
	movl	%eax,-4(%rsp)
	movsd	-8(%rsp),%xmm0
#endif
	ret
