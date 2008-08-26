
/* Public domain, Otto Moerbeek, 2007 */

#include <setjmp.h>

/*ARGSUSED*/
int
sigsetjmp(sigjmp_buf env, int savemask)
{
	return 0;
}
