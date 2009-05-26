/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: e_atan2f.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

ENTRY(atan2f)
	XMM_TWO_ARG_FLOAT_PROLOGUE
	flds	ARG_FLOAT_ONE
	flds	ARG_FLOAT_TWO
	fpatan
	XMM_FLOAT_EPILOGUE
	ret
