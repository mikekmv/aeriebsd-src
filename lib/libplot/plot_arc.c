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
#include <errno.h>
#include <plot.h>
#include "coup.h"

int
plot_arc(void *v, int x, int y, int x0, int y0, int x1, int y1)
{
	struct plotctx *pl = v;
	char *p;

	if (!(pl->caps & _PLOT_RC)) {
		pl->caps |= _PLOT_RC;
		if (cgetstr(pl->buf, "rc", &p) > 0)
			pl->rc = p;
	}

	if (pl->rc) {
		plot_move(pl, x, y);
		return (_plot_out(pl, pl->rc,
		    x0 * pl->xnum / pl->xdenom, y0 * pl->ynum / pl->ydenom,
		    x1 * pl->xnum / pl->xdenom, y1 * pl->ynum / pl->ydenom));
#if 0
	} else if (pl->flags & ARCVSLN ||
	    ((pl->caps & _PLOT_CO || cgetstr(pl->buf, "co", &p) > 0) ||
	     (pl->caps & _PLOT_LN || cgetstr(pl->buf, "ln", &p) > 0))) {

		/* TODO emulate through the line */
		flags |= ARCVSLN;
#endif
	} else {
		/* TODO emulate through the point */
		errno = EOPNOTSUPP;
		return (-1);
	}

	return (0);
}
