
/*
 * Written by J.T. Conklin, Apr 4, 1995
 * Public domain.
 */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fpgetround.S,v 1.1.1.1 2008/08/26 14:38:16 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

/*
 * XXX store only x87 state. If an application only uses the fp*
 * interface, this should be in sync with the SSE mxcsr register.
 */

#ifdef WEAK_ALIAS
WEAK_ALIAS(fpgetround, _fpgetround)
ENTRY(_fpgetround)
#else
ENTRY(fpgetround)
#endif
	fnstcw -4(%rsp)
	movl -4(%rsp),%eax
	rorl $10,%eax
	andl $3,%eax
	ret
