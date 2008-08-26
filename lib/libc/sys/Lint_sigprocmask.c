
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
sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	return (0);
}
