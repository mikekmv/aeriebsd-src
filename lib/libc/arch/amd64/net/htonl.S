
/* Written by Artur Grabowski. Public Domain */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: htonl.S,v 1.1.1.1 2008/08/26 14:38:16 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(_C_LABEL(htonl))
	movl    %edi,%eax
	bswapl  %eax
	ret
