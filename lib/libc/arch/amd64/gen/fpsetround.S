
/*
 * Written by Frank van der Linden at Wasabi Systems for NetBSD.
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.rodata
	.asciz "$ABSD$"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

/*
 * XXX set both the x87 control word and the SSE mxcsr register.
 * Applications should only set exception and round flags
 * via the fp*() interface, otherwise the status words
 * will get our of sync.
 */ 


#ifdef WEAK_ALIAS
WEAK_ALIAS(fpsetround, _fpsetround)
ENTRY(_fpsetround)
#else
ENTRY(fpsetround)
#endif
	fnstcw	-4(%rsp)
	stmxcsr	-8(%rsp)

	andl	$3,%edi

	movl	-4(%rsp),%edx
	rorl	$10,%edx
	movl	%edx,%eax
	andl	$3,%eax

	andl	$~3,%edx
	orl	%edi,%edx
	roll	$10,%edx
	movl	%edx,-4(%rsp)

	movl	-8(%rsp),%edx
	rorl	$13,%edx
	andl	$~3,%edx
	orl	%edi,%edx
	roll	$13,%edx
	movl	%edx,-8(%rsp)

	ldmxcsr	-8(%rsp)
	fldcw	-4(%rsp)
	ret