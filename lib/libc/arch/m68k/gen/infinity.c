
#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <math.h>

/* bytes for +Infinity on a 68k */
char __infinity[] = { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
