
#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: fabs.S,v 1.2 2009/05/23 01:24:04 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>

/*
 * Ok, this sucks. Is there really no way to push an xmm register onto
 * the FP stack directly?
 */

ENTRY(fabs)
	movsd	%xmm0, -8(%rsp)
	fldl	-8(%rsp)
	fabs
	fstpl	-8(%rsp)
	movsd	-8(%rsp),%xmm0
	ret
