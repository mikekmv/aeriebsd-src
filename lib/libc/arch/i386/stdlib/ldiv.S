/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: ldiv.S,v 1.1.1.1 2008/08/26 14:38:19 root Exp $"
#endif

ENTRY(ldiv)
	movl	4(%esp),%eax
	movl	8(%esp),%ecx
	cdq
	idiv	%ecx
	movl	%eax,4(%esp)
	movl	%edx,8(%esp)
	ret
