/*
 * Written by Michael Shalayeff. Public Domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include "math.h"

float
rintf(float x)
{
	__asm__ __volatile__("frnd,sgl %0,%0" : "+f" (x));

	return (x);
}
