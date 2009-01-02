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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <plot.h>
#include "coup.h"

int
_plot_out(struct plotctx *pl, const char *s, ...)
{
	va_list ap;
	long x, y, n;
	int err, rev = 0;
	char *p;

	if (!(pl->caps & _PLOT_EM)) {
		pl->caps |= _PLOT_EM;
		if (cgetstr(pl->buf, "em", &p) > 0) {
			if (!strcmp(p, "tek")) {
				pl->putxy = _plot_tekxy;
			} else if (!strcmp(p, "ascii")) {
				pl->putxy = _plot_asciixy;
			} else if (!strcmp(p, "plot")) {
				pl->putxy = _plot_plotxy;
			} else if (!strcmp(p, "dumb")) {
				pl->matrix = malloc(pl->w * pl->h);
				if (pl->matrix == NULL)
					return (-1);
				memset(pl->matrix, ' ', pl->w * pl->h);
				pl->flags |= MATRIX;
				pl->putxy = _plot_dumbxy;
				pl->puts = _plot_dumb_puts;
				pl->putfi = _plot_dumbfi;
			} else {
				/* dunno how to do it */
				errno = EOPNOTSUPP;
				return (-1);
			}
		}
	}

	/* TODO put everything into the buffer and then print that whole */
	va_start(ap, s);
	if (pl->flags & MATRIX) {
		x = va_arg(ap, long);
		y = va_arg(ap, long);
		if ((err = (pl->putxy)(pl, x, y, *s)))
			return (err);
	} else while (*s)
		switch (*s++) {
		case '%':
			switch (*s++) {
			case '%':
				if (fputc('%', pl->fp) == EOF) {
					va_end(ap);
					return (-1);
				}
				break;
			case 'c':
				if (rev)
					err = (pl->putxy)(pl, pl->y, pl->x, s[-1]);
				else
					err = (pl->putxy)(pl, pl->x, pl->y, s[-1]);
				if (err) {
					va_end(ap);
					return (err);
				}
				break;
			case 'C':
				x = va_arg(ap, long);
				y = va_arg(ap, long);
				if (rev)
					err = (pl->putxy)(pl, y, x, s[-1]);
				else
					err = (pl->putxy)(pl, x, y, s[-1]);
				if (err) {
					va_end(ap);
					return (err);
				}
				break;
			case 'd':
				n = va_arg(ap, long);
				if ((err = (pl->putxy)(pl, n, 0, s[-1]))) {
					va_end(ap);
					return (err);
				}
				break;
			case 'r':
				rev++;
				break;
			case 's':
				p = va_arg(ap, char *);
				if (pl->flags & MATRIX)
					err = (pl->puts)(pl, p);
				else
					err = fputs(p, pl->fp) == EOF? -1 : 0;
				if (err) {
					va_end(ap);
					return (err);
				}
				break;
			default:
				/* dunno -- ignore */
				break;
			}
			break;
		default:
			if (fputc(s[-1], pl->fp) == EOF) {
				va_end(ap);
				return (-1);
			}
		}
	va_end(ap);

	return (0);
}

int
_plot_asciixy(struct plotctx *pl, int x, int y, int c)
{
	fprintf(pl->fp, "%d", x);
	if (c != 'd')
		fprintf(pl->fp, " %d", y);

	return (0);
}

int
_plot_dumbxy(struct plotctx *pl, int x, int y, int c)
{
	pl->matrix[y * pl->w + x] = c;

	return (0);
}

int
_plot_dumb_puts(struct plotctx *pl, const char *s)
{
	char *p, *ep;
	long x = pl->x, y = pl->y;

	y *= pl->w;
	p = &pl->matrix[y + x];
	ep = &pl->matrix[y + pl->w];
	while(p < ep && (*p++ = *s++))
		;

	return (0);
}

int
_plot_dumbfi(struct plotctx *pl)
{
	char *p = pl->matrix + pl->w * (pl->h - 1);
	long i;

	for (i = pl->h; i--; fputc('\n', pl->fp), p -= pl->w)
		fwrite(p, sizeof(pl->matrix[0]), pl->w, pl->fp);

	return (0);
}

int
_plot_plotxy(struct plotctx *pl, int x, int y, int c)
{
	u_int16_t buf[2];

	if (pl->flags & LENDIAN)
		buf[0] = htole16(x), buf[1] = htole16(y);
	else
		buf[0] = htobe16(x), buf[1] = htobe16(y);

	fwrite(buf, sizeof(buf[0]), c == 'd'? 1 : 2, pl->fp);

	return (0);
}

int
_plot_tekxy(struct plotctx *pl, int x, int y, int c)
{
	long extra;

	/* 4013 does not support 12 bits addressing -- fake it XXX */
	if (pl->w == 780) {
		x *= 4;
		y *= 4;
	}

	extra = 0;

	if ((pl->sy ^ y) & 0xf80)
		fputc(0x20 | ((y >> 7 ) & 0x1f), pl->fp);
	if ((pl->sx ^ x) & 3 || (pl->sy ^ y) & 3) {
		fputc(0x60 | ((x & 3) | ((y & 3) << 2)), pl->fp);
		extra++;
	}
	if ((pl->sx ^ x) & 0xf80) {
		fputc(0x60 | ((y >> 2) & 0x1f), pl->fp);
		fputc(0x20 | ((x >> 7 ) & 0x1f), pl->fp);
	} else if (extra || (pl->sy ^ y) & 0x7c)
		fputc(0x60 | ((y >> 2) & 0x1f), pl->fp);
	fputc(0x40 | ((x >> 2) & 0x1f), pl->fp);
#if 0
	{
		long n = (abs(pl->sx - x) + abs(pl->sy - y) + 0x300) / (12 + 7);
		while (n--)
			fputc('\0', pl->fp);
	}
#endif
	pl->sx = x;
	pl->sy = y;

	return (0);
}
