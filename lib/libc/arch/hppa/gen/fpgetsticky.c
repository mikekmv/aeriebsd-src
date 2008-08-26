
/*
 * Written by Miodrag Vallat.  Public domain
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <sys/types.h>
#include <ieeefp.h>

fp_except
fpgetsticky()
{
	u_int64_t fpsr;

	__asm__ __volatile__("fstd %%fr0,0(%1)" : "=m" (fpsr) : "r" (&fpsr));
	return ((fpsr >> 59) & 0x1f);
}
