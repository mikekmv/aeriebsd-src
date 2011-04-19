/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#include <machine/asm.h>

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: ldiv.S,v 1.4 2011/04/19 17:43:29 mickey Exp $"
	.text
#endif

ENTRY(ldiv)
	pushl	%ebx
	movl	8(%esp),%ebx
	movl	12(%esp),%eax
	movl	16(%esp),%ecx
	cdq
	idiv	%ecx
	movl	%eax,(%ebx)
	movl	%edx,4(%ebx)
	popl	%ebx
	movl	%ebx,%eax
	ret
