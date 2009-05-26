/*
 * Written by Michael Shalayeff. Public Domain
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: s_rint.c,v 1.1.1.1 2008/08/26 14:38:51 root Exp $";
#endif

#include <sys/cdefs.h>
#include <float.h>
#include <math.h>

double
rint(double x)
{
	__asm__ __volatile__("frnd,dbl %0,%0" : "+f" (x));

	return (x);
}

#if LDBL_MANT_DIG == 53
#ifdef __weak_alias   
__weak_alias(rintl, rint);
#endif /* __weak_alias */
#endif /* LDBL_MANT_DIG == 53 */
