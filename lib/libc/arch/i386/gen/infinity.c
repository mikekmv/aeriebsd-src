
#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <math.h>

/* bytes for +Infinity on a 387 */
char __infinity[] = { 0, 0, 0, 0, 0, 0, (char)0xf0, 0x7f };
