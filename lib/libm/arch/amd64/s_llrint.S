/*
 * Written by Martynas Venckus.  Public domain
 */

#if defined(LIBM_SCCS)
	.section .rodata
	.asciz "$ABSD: s_llrint.S,v 1.1 2009/05/26 23:27:27 mickey Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

ENTRY(llrint)
	cvtsd2si %xmm0, %rax
	ret

