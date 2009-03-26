/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: rindex.S,v 1.1.1.1 2008/08/26 14:38:19 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

#ifdef STRRCHR
ENTRY(strrchr)
#else
ENTRY(rindex)
#endif
	pushl	%ebx
	movl	8(%esp),%edx
	movb	12(%esp),%cl
	xorl	%eax,%eax		/* init pointer to null */
	.align 2,0x90
L1:
	movb	(%edx),%bl
	cmpb	%bl,%cl
	jne	L2
	movl	%edx,%eax
L2:
	incl	%edx
	testb	%bl,%bl			/* null terminator??? */
	jnz	L1
	popl	%ebx
	ret
