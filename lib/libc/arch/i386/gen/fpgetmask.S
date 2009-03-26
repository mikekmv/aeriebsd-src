/*
 * Written by J.T. Conklin, Apr 4, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetmask.S,v 1.1.1.1 2008/08/26 14:38:19 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpgetmask)
	subl $4,%esp
	fnstcw (%esp)
	movl (%esp),%eax
	notl %eax
	andl $63,%eax
	addl $4,%esp
	ret
