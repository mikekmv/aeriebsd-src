/* @(#)wr_gamma.c 5.1 93/09/24 */
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
static const char rcsid[] = "$ABSD: w_gamma_r.c,v 1.1.1.1 2008/08/26 14:38:55 root Exp $";
#endif

/* 
 * wrapper double gamma_r(double x, int *signgamp)
 */

#include "math.h"
#include "math_private.h"

double
gamma_r(double x, int *signgamp) /* wrapper lgamma_r */
{
	return lgamma_r(x,signgamp);
}             
