
/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

flag __ltsf2(float32, float32);

flag
__ltsf2(float32 a, float32 b)
{

	/* libgcc1.c says -(a < b) */
	return -float32_lt(a, b);
}
