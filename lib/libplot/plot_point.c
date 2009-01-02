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
#include <plot.h>
#include <errno.h>
#include "coup.h"

int
plot_point(void *v, int x, int y)
{
	struct plotctx *pl = v;
	char *p;
	int err;

        if (x < pl->ox || x >= (pl->ox + pl->dx) ||
	    y < pl->oy || y >= (pl->oy + pl->dy))
		return (0);

	if (!(pl->caps & _PLOT_PT)) {
		if (cgetstr(pl->buf, "pt", &p) > 0) {
			pl->pt = p;
			pl->caps |= _PLOT_PT;
		}
	}

	if (pl->pt) {
		long rx, ry;
		rx = (x - pl->ox) * pl->xnum / pl->xdenom;
		ry = (y - pl->oy) * pl->ynum / pl->ydenom;

		if ((err = _plot_out(pl, pl->pt, rx, ry)))
			return (err);
	} else if (pl->flags & PTVSLN || cgetstr(pl->buf, "ln", &p) > 0 ||
	    cgetstr(pl->buf, "co", &p) > 0) {
		pl->flags |= PTVSLN;
		return (plot_line(v, x, y, x, y));
	} else if (pl->flags & PTVSCI || cgetstr(pl->buf, "ci", &p) > 0) {
		pl->flags |= PTVSCI;
		return (plot_circle(v, x, y, 1));
	} else if (pl->flags & PTVSDT || cgetstr(pl->buf, "dt", &p) > 0) {
		pl->flags |= PTVSDT;
		return (plot_dot(v, x, y, x, y, NULL));
	} else {
		/* CAN'T MAKE A POINT */
		errno = EOPNOTSUPP;
		return -1;
	}

	pl->x = x;
	pl->y = y;

	return (0);
}
