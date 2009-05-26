/*
 * Written by Michael Shalayeff. Public Domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: e_remainderf.c,v 1.1.1.1 2008/08/26 14:38:51 root Exp $";
#endif

#include "math.h"

float
remainderf(float x, float p)
{
	__asm__ __volatile__("frem,sgl %0,%1,%0" : "+f" (x) : "f" (p));

	return (x);
}
