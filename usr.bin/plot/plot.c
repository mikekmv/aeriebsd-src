/*
 * Copyright (c) 2005 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef lint
static const char rcsid[] = "$ABSD: plot.c,v 1.4 2009/05/28 13:53:35 mickey Exp $";
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <plot.h>
#include <err.h>

#define	STRBUFSZ	132

void
usage()
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-T terminal] [-r resolution] [file] ...\n",
	    __progname);
}

const struct linmod {
	int id;
	char	name[12];
} linmods[] = {
	{ 1, "solid" },
	{ 2, "dotted" },
	{ 3, "longdashed" },
	{ 4, "shortdashed" },
	{ 5, "dotdashed" },
	{ 0, "" }
};

int
getint(const char *fname)
{
	int a, b;

	if ((a = getchar()) == EOF) {
		if (ferror(stdin))
			err(1, "getchar: %s", fname);
		else
			errx(1, "%s: unexpected EOF", fname);
	}

	if ((b = getchar()) == EOF) {
		if (ferror(stdin))
			err(1, "getchar: %s", fname);
		else
			errx(1, "%s: unexpected EOF", fname);
	}
	return (b << 8) | a;
}

int
main(int argc, char *argv[])
{
	char buf[STRBUFSZ], *q;
	const struct linmod *lm;
	int ch, ch2, xy[8], *p;
	char *fname, *term = NULL;
	void *v;

	while ((ch = getopt(argc, argv, "r:T:")) != -1)
		switch (ch) {
		case 'T':
			term = optarg;
			break;

		case 'r':
			break;

		default:
			usage();
			break;
		}

	argc -= optind;
	argv += optind;

	fname = "<stdin>";
	do {
		if (*argv) {
			fname = *argv;
			argv++;
			if (!(freopen(fname, "r", stdin)))
				err(1, "freopen: %s", fname);
		}

		if (!(v = plot_open(term, "dumb", stdout)))
			err(1, "plot_open");

		while((ch = getchar()) != EOF) {

			p = xy;
			switch (ch) {
			case 'a':	/* arc */
				*p++ = getint(fname);
				*p++ = getint(fname);
				/* FALLTHROUGH */
			case 'l':	/* line */
			case 's':	/* space */
				*p++ = getint(fname);
				*p++ = getint(fname);
				/* FALLTHROUGH */
			case 'c':	/* circle */
			case 'm':	/* move */
			case 'n':	/* continue */
			case 'p':	/* point */
				*p++ = getint(fname);
				*p++ = getint(fname);
				if (ch == 'c')
					*p++ = getint(fname);
				break;

			case 'f':	/* line mode */
			case 't':	/* label */
				for (ch2 = 0, q = buf; q < &buf[STRBUFSZ] &&
				    (ch2 = getchar()) != EOF &&
				    ch2 != '\n'; *q++ = ch2)
					;
				if (ferror(stdin))
					err(1, "getchar: %s", fname);
				if (ch2 != '\n')
					errx(1, "error reading label");
				*q = '\0';
				break;

			case 'e':	/* erase */
				break;

			default:
				errx(1, "invalid input \'%c\' (%d)", ch, ch);
			}

			switch (ch) {
			case 'a':
				if (plot_arc(v, xy[0], xy[1], xy[2], xy[3],
				    xy[4], xy[5]))
					err(1, "arc");
				break;

			case 'c':
				if (plot_circle(v, xy[0], xy[1], xy[2]))
					err(1, "circle");
				break;

			case 'e':
				if (plot_erase(v))
					err(1, "erase");
				break;

			case 'f':
				for (lm = linmods; lm->id != 0 &&
				    strcasecmp(buf, lm->name); lm++)
					;
				if (lm->id == 0)
					warnx("invalid line mode \'%s\'", buf);
				plot_linemod(v, ch);
				break;

			case 'l':
				if (plot_line(v, xy[0], xy[1], xy[2], xy[3]))
					err(1, "line");
				break;

			case 'm':
				if (plot_move(v, xy[0], xy[1]))
					err(1, "move");
				break;

			case 'n':
				if (plot_cont(v, xy[0], xy[1]))
					err(1, "cont");
				break;

			case 'p':
				if (plot_point(v, xy[0], xy[1]))
					err(1, "point");
				break;

			case 's':
				if (plot_space(v, xy[0], xy[1], xy[2], xy[3]))
					err(1, "space");
				break;

			case 't':
				if (plot_label(v, buf))
					err(1, "label");
				break;
			}
		}

		if (ferror(stdin))
			err(1, "%s", fname);

		if (plot_close(v))
			err(1, "plot_close");

	} while(*argv);

	return (0);
}
