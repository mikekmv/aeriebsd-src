
/* Written by Artur Grabowski. Public Domain */

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: ntohl.S,v 1.2 2009/05/23 01:24:05 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>

ENTRY(_C_LABEL(ntohl))
	movl    %edi,%eax
	bswapl  %eax
	ret
