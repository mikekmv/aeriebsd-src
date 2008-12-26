/* Written by Martynas Venckus.  Public Domain. */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <math.h>

/* bytes for qNaN on an amd64 (IEEE single format) */
char __nan[] __attribute__((__aligned__(sizeof(float)))) =
					{ 0, 0, 0xc0, 0x7f };
