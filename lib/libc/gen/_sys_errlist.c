/*
 * Written by J.T. Conklin, December 12, 1994
 * Public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: _sys_errlist.c,v 1.1.1.1 2008/08/26 14:38:27 root Exp $";
#endif

#include <sys/cdefs.h>

#ifdef __indr_reference
__indr_reference(_sys_errlist, sys_errlist);
__indr_reference(_sys_errlist, __sys_errlist); /* Backwards compat with v.12 */
#else

#undef _sys_errlist
#undef _sys_nerr
#define	_sys_errlist	sys_errlist
#define	_sys_nerr	sys_nerr
#undef LIBC_SCCS
#include "errlist.c"

#undef _sys_errlist
#undef _sys_nerr
#define	_sys_errlist	__sys_errlist
#define	_sys_nerr	__sys_nerr
#undef LIBC_SCCS
#include "errlist.c"

#endif
