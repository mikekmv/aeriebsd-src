/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD$"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

ENTRY(rintf)
	XMM_ONE_ARG_FLOAT_PROLOGUE
	flds	ARG_FLOAT_ONE
	frndint
	XMM_FLOAT_EPILOGUE
	ret