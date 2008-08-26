
/* Public domain, Otto Moerbeek, 2007 */

#include <setjmp.h>

/*ARGSUSED*/
int
setjmp(jmp_buf env)
{
	return 0;
}

/*ARGSUSED*/
int
_setjmp(jmp_buf env)
{
	return 0;
}
