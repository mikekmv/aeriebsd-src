
/* Written by Artur Grabowski. Public Domain */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: ntohs.S,v 1.1.1.1 2008/08/26 14:38:16 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(_C_LABEL(ntohs))
	movl    %edi,%eax
	xchgb   %ah,%al
	ret

