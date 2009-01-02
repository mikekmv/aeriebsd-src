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
#include "coup.h"

int
plot_cont(void *v, int x, int y)
{
	struct plotctx *pl = v;
	char *p;
	int err;

	if (x < pl->ox)
		x = pl->ox;
	if (x >= (pl->ox + pl->dx))
		x = pl->ox + pl->dx - 1;
	if (y < pl->oy)
		y = pl->oy;
	if (y >= (pl->oy + pl->dy))
		y = pl->oy + pl->dy - 1;

	if (!(pl->caps & _PLOT_CO)) {
		pl->caps |= _PLOT_CO;
		if (cgetstr(pl->buf, "co", &p) > 0) {
			pl->co = p;
		}
	}

	if (pl->co) {
		long rx, ry;

		rx = (x - pl->ox) * pl->xnum / pl->xdenom;
		ry = (y - pl->oy) * pl->ynum / pl->ydenom;

		if ((err = _plot_out(pl, pl->co, rx, ry)))
			return (err);
	} else if (pl->flags & COVSLN || cgetstr(pl->buf, "ln", &p) > 0) {
		pl->flags |= COVSLN;
		if ((err = plot_line(pl, pl->x, pl->y, x, y)))
			return (err);
	} else {
		/* assume a priori of being able to make a point */
		int xi, dx, dx1, dx2, yi, dy, dy1, dy2, n, e, s, a;

		xi = pl->x, yi = pl->y;
		if (x >= xi)
			dx1 = dx2 = 1, dx = x - xi;
		else
			dx1 = dx2 = -1, dx = xi - x;
		if (y >= yi)
			dy1 = dy2 = 1, dy = y - yi;
		else
			dy1 = dy2 = -1, dy = yi - y;
		if (dx >= dy)
			dx1 = dy2 = 0, a = dx, e = dx / 2, s = dy, n = dx;
		else
			dx2 = dy1 = 0, a = dy, e = dy / 2, s = dx, n = dy;

		while (n--) {
			if ((err = plot_point(pl, xi, yi)))
				return (err);
			e += s;
			if (e >= a)
				e -= a, xi += dx1, yi += dy1;
			xi += dx2, yi += dy2;
		}
	}

	pl->x = x;
	pl->y = y;

	return (0);
}
