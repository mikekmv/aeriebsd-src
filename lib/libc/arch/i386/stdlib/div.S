/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#include <machine/asm.h>

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: div.S,v 1.3 2009/10/31 21:30:54 mickey Exp $"
	.text
#endif

ENTRY(div)
	pushl	%ebx
	movl	8(%esp),%ebx
	movl	12(%esp),%eax
	movl	16(%esp),%ecx
	cdq
	idiv	%ecx
	movl	%eax,(%ebx)
	movl	%edx,4(%ebx)
	movl	%ebx,%eax
	popl	%ebx
	ret
