/* written by Michael Shalayeff, 2009. Public Domain */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#define	DSET	(1024 * 1024)

int
main(int argc, char **argv)
{
	char boo[64 * 1024], *ptr;
	struct rlimit rlim;
	int res, oe;

	if (argc > 1 && *argv[1] == 's') {
		memset(boo, 0, sizeof boo);
		res = RLIMIT_STACK;
	} else {
		ptr = malloc(DSET);
		memset(ptr, 0, DSET);
		res = RLIMIT_DATA;
	}

	errno = 0;
	/* force limit below current use */
	rlim.rlim_cur = 32 * 1024;
	rlim.rlim_max = 32 * 1024;
	if (setrlimit(res, &rlim) < 0 && errno == EINVAL)
		return 0;

	oe = errno;
	if (res == RLIMIT_DATA) {
		memset(ptr, 0, DSET);
		/* if we still did not fault see if malloc() fails now */
		if ((ptr = malloc(DSET))) {
			/* we will err() later anyway */
			warnx("malloc() did not fail");
			memset(ptr, 0, DSET);
		}
	}

	/* we most likely should have had SIGSEGV and failed anyway */
	errno = oe;
	err(1, "setrlimit");
}
