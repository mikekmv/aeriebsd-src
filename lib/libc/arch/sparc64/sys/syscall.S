/*
 * Placed under public domain by art@openbsd.org
 */
#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: syscall.S,v 1.2 2009/05/23 01:24:15 mickey Exp $"
	.text
#endif /* LIBC_SCCS */

#include <machine/asm.h>
#include "SYS.h"

RSYSCALL(syscall)
