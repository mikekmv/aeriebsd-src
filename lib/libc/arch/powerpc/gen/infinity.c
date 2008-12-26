
#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <math.h>

/* bytes for +Infinity on a PowerPC */
char __infinity[] __attribute__((__aligned__(sizeof(double)))) =
    { 0x7f, (char)0xf0, 0, 0, 0, 0, 0, 0 };
