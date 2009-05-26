
/*
 * Written by Miodrag Vallat.  Public domain
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: fabs.c,v 1.1.1.1 2008/08/26 14:38:18 root Exp $";
#endif

#include <sys/cdefs.h>

double
fabs(double val)
{

	__asm__ __volatile__("fabs,dbl %0,%0" : "+f" (val));
	return (val);
}

__weak_alias(fabsl, fabs);
