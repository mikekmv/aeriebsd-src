
/*
 * IEEE-compatible infinity.c -- public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <sys/types.h>
#include <math.h>
#include <machine/endian.h>

char __infinity[] __attribute__((__aligned__(sizeof(double)))) =
#if BYTE_ORDER == BIG_ENDIAN
	{ 0x7f, 0xf0,    0,    0, 0, 0,    0,    0};
#else
#ifdef __VFP_FP__
	{    0,    0,    0,    0, 0, 0, 0xf0, 0x7f};
#else
	{    0,    0, 0xf0, 0x7f, 0, 0,    0,    0};
#endif
#endif
