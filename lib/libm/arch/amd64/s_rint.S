/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_rint.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

ENTRY(rint)
	XMM_ONE_ARG_DOUBLE_PROLOGUE
	fldl	ARG_DOUBLE_ONE
	frndint
	XMM_DOUBLE_EPILOGUE
	ret
