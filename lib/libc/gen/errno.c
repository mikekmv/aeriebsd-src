/* PUBLIC DOMAIN: No Rights Reserved.   Marco S Hyman <marc@snafu.org> */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <errno.h>
#undef errno

/*
 * global errno for unthreaded programs.
 */

int errno;

/*
 * weak version of function used by unthreaded programs.
 */
int *
___errno(void)
{
	return &errno;
}

__weak_alias(__errno, ___errno);
