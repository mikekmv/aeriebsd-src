/* Written by Martynas Venckus.  Public Domain. */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <math.h>

/* bytes for qNaN on a sparc64 (IEEE single format) */
char __nan[] __attribute__((__aligned__(sizeof(float)))) =
					{ 0x7f, 0xc0, 0, 0 };
