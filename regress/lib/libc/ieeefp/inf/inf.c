
/*
 * Peter Valchev <pvalchev@openbsd.org> Public Domain, 2002.
 */

#include <math.h>

int
main(int argc, char *argv[])
{
	if (isinf(HUGE_VAL))
		return 0;

	return 1;
}
