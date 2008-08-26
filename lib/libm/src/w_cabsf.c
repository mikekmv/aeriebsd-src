/*
 * cabsf() wrapper for hypotf().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include "math.h"
#include "math_private.h"

struct complex {
	float x;
	float y;
};

float
cabsf(struct complex z)
{
	return hypotf(z.x, z.y);
}
