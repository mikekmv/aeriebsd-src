
/* Written by Artur Grabowski. Public Domain */

#if defined(LIBC_SCCS)
	.rodata
	.asciz "$ABSD$"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(_C_LABEL(ntohs))
	movl    %edi,%eax
	xchgb   %ah,%al
	ret

