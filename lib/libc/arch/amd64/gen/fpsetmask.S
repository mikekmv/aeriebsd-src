
/*
 * Written by Frank van der Linden at Wasabi Systems for NetBSD.
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpsetmask.S,v 1.2 2009/05/23 01:24:05 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>

/*
 * XXX set both the x87 control word and the SSE mxcsr register.
 * Applications should only set exception and round flags
 * via the fp*() interface, otherwise the status words
 * will get our of sync.
 */

#ifdef WEAK_ALIAS
WEAK_ALIAS(fpsetmask, _fpsetmask)
ENTRY(_fpsetmask)
#else
ENTRY(fpsetmask)
#endif
	fnstcw	-4(%rsp)
	stmxcsr	-8(%rsp)
	andl	$63,%edi
	notl	%edi

	movl	-4(%rsp),%edx
	movl	%edx,%eax
	andl	%edi,%edx
	movl	%edx,-4(%rsp)

	movl	-8(%rsp),%edx
	roll	$7,%edi
	andl	%edi,%edx
	movl	%edx,-8(%rsp)

	fldcw	-4(%rsp)
	ldmxcsr	-8(%rsp)
	andl	$63,%eax
	ret
