/*
 * Written by Martynas Venckus.  Public domain
 */

#if defined(LIBM_SCCS)
	.section .rodata
	.asciz "$ABSD: s_lrint.S,v 1.1 2009/05/26 23:27:27 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(lrint)
	cvtsd2si %xmm0, %rax
	ret

