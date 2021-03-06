/* @(#)s_fabs.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: s_fabs.c,v 1.1.1.1 2008/08/26 14:38:54 root Exp $";
#endif

/*
 * fabs(x) returns the absolute value of x.
 */

#include <sys/cdefs.h>
#include <float.h>
#include <math.h>

#include "math_private.h"

double
fabs(double x)
{
	u_int32_t high;
	GET_HIGH_WORD(high,x);
	SET_HIGH_WORD(x,high&0x7fffffff);
        return x;
}

#if LDBL_MANT_DIG == 53
#ifdef __weak_alias
__weak_alias(fabsl, fabs);
#endif /* __weak_alias */
#endif /* LDBL_MANT_DIG == 53 */
