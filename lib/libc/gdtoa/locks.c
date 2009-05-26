/* Written by Martynas Venckus.  Public Domain. */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <stdio.h>

void *__dtoa_locks[] = { NULL, NULL };
