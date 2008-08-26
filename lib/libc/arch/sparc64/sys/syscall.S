/*
 * Placed under public domain by art@openbsd.org
 */
#if defined(LIBC_SCCS)
	.rodata
	.asciz "$ABSD$"
#endif /* LIBC_SCCS */

#include <machine/asm.h>
#include "SYS.h"

RSYSCALL(syscall)
