/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

#if defined(LIBM_SCCS)
	.text
	.asciz "$ABSD: s_tan.S,v 1.1.1.1 2008/08/26 14:38:51 root Exp $"
	.text
#endif /* LIBM_SCCS */

#include <machine/asm.h>

#include "abi.h"

ENTRY(tan)
	XMM_ONE_ARG_DOUBLE_PROLOGUE
	fldl	ARG_DOUBLE_ONE
	fptan
	fnstsw	%ax
	andw	$0x400,%ax
	jnz	1f
	fstp	%st(0)
	XMM_DOUBLE_EPILOGUE
	ret
1:	fldpi
	fadd	%st(0)
	fxch	%st(1)
2:	fprem1
	fstsw	%ax
	andw	$0x400,%ax
	jnz	2b
	fstp	%st(1)
	fptan
	fstp	%st(0)
	XMM_DOUBLE_EPILOGUE
	ret
