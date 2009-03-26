/*
 * Written by J.T. Conklin, December 12, 1994
 * Public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: _sys_siglist.c,v 1.1.1.1 2008/08/26 14:38:27 root Exp $";
#endif

#include <sys/cdefs.h>

#ifdef __indr_reference
__indr_reference(_sys_siglist, sys_siglist);
__indr_reference(_sys_siglist, __sys_siglist); /* Backwards compat with v.12 */
#else

#undef _sys_siglist
#define	_sys_siglist	sys_siglist
#undef LIBC_SCCS
#include "siglist.c"

#undef _sys_siglist
#define	_sys_siglist	__sys_siglist
#undef LIBC_SCCS
#include "siglist.c"

#endif
