
/* Public domain, Otto Moerbeek, 2007 */

#include <setjmp.h>

/*ARGSUSED*/
void
longjmp(jmp_buf env, int val)
{
}

/*ARGSUSED*/
void
_longjmp(jmp_buf env, int val)
{
}
