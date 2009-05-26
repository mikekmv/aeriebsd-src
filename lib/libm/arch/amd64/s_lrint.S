/*
 * Written by Martynas Venckus.  Public domain
 */

#if defined(LIBM_SCCS)
	.section .rodata
	.asciz "$ABSD$"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(lrint)
	cvtsd2si %xmm0, %rax
	ret

