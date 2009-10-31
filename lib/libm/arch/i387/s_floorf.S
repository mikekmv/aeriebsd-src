/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_floorf.S,v 1.1.1.1 2008/08/26 14:38:52 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(floorf)
	subl	$8,%esp

	fstcw	4(%esp)		/* store fpu control word */
	movw	4(%esp),%dx
	orw	$0x0400,%dx		/* round towards -oo */
	andw	$0xf7ff,%dx
	movw	%dx,(%esp)
	fldcw	(%esp)		/* load modfied control word */

	flds	12(%esp);		/* round */
	frndint

	fldcw	4(%esp)		/* restore original control word */

	addl	$8,%esp
	ret
