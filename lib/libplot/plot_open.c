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

#if defined(LIBPLOT_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <plot.h>
#include "coup.h"

void *
plot_open(const char *name, const char *def, FILE *fp)
{
	struct plotctx *pl;
	char *p, *dba[2];
	long n;
	int err, ontty;

	ontty = isatty(fileno(fp));
	if (name == NULL && ontty && (p = getenv("PLOTCAP")) != NULL) {
		if ((p = strdup(p)) == NULL)
			return (NULL);
	} else {
		dba[0] = _PATH_PLOTCAP;
		dba[1] = NULL;
		if (name == NULL) {
			if (ontty)
				name = getenv("TERM");
			if (name == NULL)
				name = def;
			if (name == NULL)
				return (NULL);
		}
		if (cgetent(&p, dba, name) != 0) {
			if (cgetent(&p, dba, def) != 0)
				return (NULL);
			else
				name = def;
		}
	}

	if ((pl = malloc(sizeof(*pl))) == NULL) {
		free(p);
		return (NULL);
	}
	memset(pl, 0, sizeof(*pl));
	pl->buf = p;

	if (cgetnum(pl->buf, "w", &n) == 0)
		pl->dx = pl->w = n;

	if (cgetnum(pl->buf, "h", &n) < 0)
		pl->dx = pl->w = 0;
	else
		pl->dx = pl->h = n;

	if (cgetcap(pl->buf, "le", ':'))
		pl->flags |= LENDIAN;

	/* scale at 1:1 */
	pl->ox = pl->oy = 0;
	pl->xnum = pl->xdenom = pl->ynum = pl->ydenom = 1;

	pl->fp = fp? fp : stdout;

	if (cgetstr(pl->buf, "in", &p) > 0)
		if (_plot_out(pl, p)) {
			err = errno;
			free(pl->buf);
			free(pl);
			errno = err;
			return (NULL);
		}

	return (pl);
}
