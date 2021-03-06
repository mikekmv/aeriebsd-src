/*
 * Copyright (c) 2009 Michael Shalayeff
 * Copyright (c) 2009 Pedro Martelletto
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "mntopts.h"

const struct mntopt mntopts[] = { MOPT_STDOPTS, { NULL } };

__dead void
usage(void)
{
	extern char *__progname;
	fprintf(stderr, "usage: %s [-o options] partition directory\n",
	    __progname);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	char mntpoint[MAXPATHLEN];
	int ch, mntflags;
	struct milfs_args args;

	mntflags = 0;

	while ((ch = getopt(argc, argv, "o:")) != -1)
		switch (ch) {
		case 'o':
			getmntopts(optarg, mntopts, &mntflags);
			break;
		default:
			usage();
	}

	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	args.fspec = argv[0];

	if (realpath(argv[1], mntpoint) == NULL)
		err(1, "realpath %s", argv[1]);

	if (mount(MOUNT_MILFS, mntpoint, mntflags, &args) < 0)
		err(1, "mount");

	exit(0);
}
