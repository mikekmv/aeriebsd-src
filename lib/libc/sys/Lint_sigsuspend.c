
/*
 * This file placed in the public domain.
 * Chris Demetriou, November 5, 1997.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <signal.h>

/*ARGSUSED*/
int
sigsuspend(const sigset_t *sigmask)
{
	return (0);
}
