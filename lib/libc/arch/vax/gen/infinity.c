/*
 * XXX - This is not correct, but what can we do about it?
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

/* infinity.c */

#include <math.h>

/* The highest D float on a vax. */
char __infinity[] = { (char)0xff, (char)0x7f, (char)0xff, (char)0xff, 
	(char)0xff, (char)0xff, (char)0xff, (char)0xff };
