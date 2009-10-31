/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: index.S,v 1.2 2009/03/26 17:30:47 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>

#ifdef STRCHR
ENTRY(strchr)
#else
ENTRY(index)
#endif
	movl	4(%esp),%eax
	movb	8(%esp),%cl
	.align 2,0x90
L1:
	movb	(%eax),%dl
	cmpb	%dl,%cl			/* found char??? */
	je 	L2
	incl	%eax
	testb	%dl,%dl			/* null terminator??? */
	jnz	L1
	xorl	%eax,%eax
L2:
	ret
