/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include "rand48.h"

extern unsigned short __rand48_seed[3];

long
mrand48(void)
{
	__dorand48(__rand48_seed);
	return ((long) __rand48_seed[2] << 16) + (long) __rand48_seed[1];
}
