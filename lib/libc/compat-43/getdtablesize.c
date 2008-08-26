/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <unistd.h>

int
getdtablesize(void)
{
	return sysconf(_SC_OPEN_MAX);
}
