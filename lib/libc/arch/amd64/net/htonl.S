
/* Written by Artur Grabowski. Public Domain */

#if defined(LIBC_SCCS)
	.rodata
	.asciz "$ABSD$"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(_C_LABEL(htonl))
	movl    %edi,%eax
	bswapl  %eax
	ret
