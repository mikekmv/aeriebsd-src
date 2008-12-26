/* Written by Martynas Venckus.  Public Domain. */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <sys/types.h>
#include <math.h>

/* bytes for qNaN on a sh (IEEE single format) */
char __nan[] __attribute__((__aligned__(sizeof(float)))) =
#if BYTE_ORDER == BIG_ENDIAN
					{ 0x7f, 0xa0, 0, 0 };
#else /* BYTE_ORDER == BIG_ENDIAN */
					{ 0, 0, 0xa0, 0x7f };
#endif /* BYTE_ORDER == BIG_ENDIAN */
