/*
 * Written by Michael Shalayeff. Public Domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: e_remainder.c,v 1.1.1.1 2008/08/26 14:38:51 root Exp $";
#endif

#include "math.h"

double
remainder(double x, double p)
{
	__asm__ __volatile__("frem,dbl %0,%1,%0" : "+f" (x) : "f" (p));

	return (x);
}
