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
plot_move(void *v, int x, int y)
{
	struct plotctx *pl = v;
	char *p;
	int err;

	if (!(pl->caps & _PLOT_MV)) {
		pl->caps |= _PLOT_MV;
		if (cgetstr(pl->buf, "mv", &p) > 0)
			pl->mv = p;
	}

	if (pl->mv) {
		long rx, ry;

		rx = (x - pl->ox) * pl->xnum / pl->xdenom;
		ry = (y - pl->oy) * pl->ynum / pl->ydenom;

		if ((err = _plot_out(pl, pl->mv, rx, ry)))
			return (err);
	}

	pl->x = x;
	pl->y = y;

	return (0);
}
