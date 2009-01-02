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
plot_linemod(void *v, int mod)
{
	struct plotctx *pl = v;
	char buf[4];
	char *p;

	if (mod > 8) {
		errno = EINVAL;
		return (-1);
	}

	snprintf(buf, sizeof(buf), "lm%d", mod);
	if (!(pl->caps & (_PLOT_LM0 << mod))) {
		pl->caps |= (_PLOT_LM0 << mod);
		if (cgetstr(pl->buf, buf, &p) > 0)
			pl->lm[mod] = p;
	}

	if (pl->lm[mod]) {
		pl->linemod = mod;
		return _plot_out(pl, pl->lm[mod]);
	} else {
		errno = EINVAL;
		return (-1);
	}
}
