
/*
 * Public domain. 2002, Federico Schwindt <fgsch@openbsd.org>.
 */

#include <err.h>
#include "defs.h"

int
main(int argc, char **argv)
{
	if (weak_func() != WEAK_REF)
		errx(1, "error calling weak reference");

	if (func() != STRONG_REF)
		errx(1, "error calling strong reference");

	return (0);
}
