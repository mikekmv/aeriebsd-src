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
plot_close(void *v)
{
	struct plotctx *pl = v;
	FILE *fp = pl->fp;
	char *p;

	if (pl->putfi)
		(pl->putfi)(pl);

	if (cgetstr(pl->buf, "fi", &p) > 0)
		_plot_out(pl, p);

	free(pl->buf);
	free(pl);
	return fflush(fp);
}
