
/*
 * Written by Frank van der Linden at Wasabi Systems for NetBSD.
 * Public domain.
 *
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetsticky.S,v 1.1.1.1 2008/08/26 14:38:16 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

/*
 * XXX read both the x87 and SSE status words here, and OR
 * them to get a complete picture of exceptions.
 */

#ifdef WEAK_ALIAS
WEAK_ALIAS(fpgetsticky, _fpgetsticky)
ENTRY(_fpgetsticky)
#else
ENTRY(fpgetsticky)
#endif
	fnstsw	-4(%rsp)
	stmxcsr	-8(%rsp)
	movl	-4(%rsp),%eax
	orl	-8(%rsp),%eax
	andl	$63,%eax
	ret
