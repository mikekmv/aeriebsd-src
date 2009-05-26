/* Written by Martynas Venckus.  Public Domain. */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: nan.c,v 1.1 2008/12/26 18:50:30 mickey Exp $";
#endif

#include <sys/types.h>
#include <math.h>

/* bytes for qNaN on a mips64 (IEEE single format) */
char __nan[] __attribute__((__aligned__(sizeof(float)))) =
#if BYTE_ORDER == BIG_ENDIAN
					{ 0x7f, 0xc0, 0, 0 };
#else /* BYTE_ORDER == BIG_ENDIAN */
					{ 0, 0, 0xc0, 0x7f };
#endif /* BYTE_ORDER == BIG_ENDIAN */
