/*
 * Written by J.T. Conklin, Apr 4, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetsticky.S,v 1.1.1.1 2008/08/26 14:38:19 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetsticky)
	subl $4,%esp
	fnstsw (%esp)
	movl (%esp),%eax
	andl $63,%eax
	addl $4,%esp
	ret
