/*
 * Placed under public domain by art@openbsd.org
 */
#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: syscall.S,v 1.1.1.1 2008/08/26 14:38:25 root Exp $"
#endif /* LIBC_SCCS */

#include <machine/asm.h>
#include "SYS.h"

RSYSCALL(syscall)
