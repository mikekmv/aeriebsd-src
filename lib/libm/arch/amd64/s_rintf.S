/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_rintf.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

ENTRY(rintf)
	XMM_ONE_ARG_FLOAT_PROLOGUE
	flds	ARG_FLOAT_ONE
	frndint
	XMM_FLOAT_EPILOGUE
	ret
