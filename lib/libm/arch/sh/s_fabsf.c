/*
 * Written by Martynas Venckus.  Public domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <math.h>

float
fabsf(float f)
{
	/* Same operation is performed regardless of precision. */
	__asm__ __volatile__ ("fabs %0" : "+f" (f));

	return (f);
}

