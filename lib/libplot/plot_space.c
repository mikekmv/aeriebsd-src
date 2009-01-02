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
#include <limits.h>
#include <plot.h>
#include "coup.h"

int
plot_space(void *v, int x0, int y0, int x1, int y1)
{
	struct plotctx *pl = v;
	char *p;
	int err;

	if (!(pl->caps & _PLOT_SP)) {
		pl->caps |= _PLOT_SP;
		if (cgetstr(pl->buf, "sp", &p) > 0)
			pl->sp = p;
	}

	if (pl->sp) {
		if ((err = _plot_out(pl, pl->sp, x0, y0, x1, y1)))
			return (err);

		pl->dx = pl->w = abs(x1 - x0);
		pl->dy = pl->h = abs(y1 - y0);
		pl->ox = pl->oy = 0;
		pl->xnum = pl->xdenom = pl->ynum = pl->ydenom = 1;
	} else {
		long num, denom, delta, mdelta;
		/* TODO recalc the current x,y */

#define	eu(d1,d2,n,d) \
for (denom = 1, mdelta = LONG_MAX; denom < d2; denom++) {	\
	num = (d1 * denom) / d2;				\
	delta = d2 * num / denom - d1;				\
	if (!delta) {						\
		(n) = num, (d) = denom;				\
		break;						\
	} else if (delta < mdelta)				\
		(n) = num, (d) = denom, mdelta = delta;		\
}

		pl->ox = x0; pl->dx = x1 - x0;
		pl->oy = y0; pl->dy = y1 - y0;
		eu(pl->w, pl->dx, pl->xnum, pl->xdenom);
		eu(pl->h, pl->dy, pl->ynum, pl->ydenom);
	}

	return (0);
}
