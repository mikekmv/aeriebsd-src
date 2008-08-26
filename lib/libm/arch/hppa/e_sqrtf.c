/*
 * Written by Michael Shalayeff. Public Domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include "math.h"

float
__ieee754_sqrtf(float x)
{
	__asm__ __volatile__ ("fsqrt,sgl %0, %0" : "+f" (x));
	return (x);
}
