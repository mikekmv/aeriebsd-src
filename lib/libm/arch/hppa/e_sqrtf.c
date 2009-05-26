/*
 * Written by Michael Shalayeff. Public Domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: e_sqrtf.c,v 1.1.1.1 2008/08/26 14:38:51 root Exp $";
#endif

#include "math.h"

float
sqrtf(float x)
{
	__asm__ __volatile__ ("fsqrt,sgl %0, %0" : "+f" (x));
	return (x);
}
