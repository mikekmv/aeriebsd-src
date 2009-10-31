/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_tanf.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

/* A float's domain isn't large enough to require argument reduction. */
ENTRY(tanf)
	XMM_ONE_ARG_FLOAT_PROLOGUE
	flds	ARG_FLOAT_ONE
	fptan
	fstp	%st(0)
	XMM_FLOAT_EPILOGUE
	ret
