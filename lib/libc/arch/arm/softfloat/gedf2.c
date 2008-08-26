
/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

flag __gedf2(float64, float64);

flag
__gedf2(float64 a, float64 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float64_le(b, a) - 1;
}
