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

#define	_PATH_PLOTCAP	"/usr/share/misc/plotcap"

void *plot_open(const char *, const char *, FILE *);
int plot_close(void *);
int plot_erase(void *);
int plot_arc(void *, int, int, int, int, int, int);
int plot_box(void *, int, int, int, int);
int plot_circle(void *, int, int, int);
int plot_cont(void *, int, int);
int plot_dot(void *, int, int, int, int, const int *);
int plot_label(void *, const char *);
int plot_line(void *, int, int, int, int);
int plot_linemod(void *, int);
int plot_move(void *, int, int);
int plot_point(void *, int, int);
int plot_space(void *, int, int, int, int);
