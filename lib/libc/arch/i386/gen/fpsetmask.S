/*
 * Written by Charles M. Hannum, Apr 9, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpsetmask.S,v 1.1.1.1 2008/08/26 14:38:19 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(fpsetmask)
	subl $4,%esp

	fnstcw (%esp)
	movl (%esp),%eax
	movl %eax,%edx

	notl %eax
	andl $63,%eax

	addl %eax,%edx
	movl 8(%esp),%ecx
	andl $63,%ecx
	subl %ecx,%edx
	movl %edx,(%esp)
	fldcw (%esp)

	addl $4,%esp
	ret
