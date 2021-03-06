/*
 * Written by J.T. Conklin, December 12, 1994
 * Public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <sys/cdefs.h>

#ifdef __indr_reference
__indr_reference(_sys_nerr, sys_nerr);
__indr_reference(_sys_nerr, __sys_nerr); /* Backwards compat with v.12 */
#endif
