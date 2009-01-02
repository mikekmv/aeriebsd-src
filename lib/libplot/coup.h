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

#define	MATRIX		0x000001
#define	LENDIAN		0x000004
#define	SCALEMU		0x000008
#define	PTVSLN		0x000010
#define	PTVSCI		0x000020
#define	PTVSDT		0x000040
#define	COVSLN		0x000080
#define	LNVSCO		0x000100
#define	ARCVSLN		0x000200
#define	LBLVSLN		0x000400

#define	_PLOT_CI	0x000001
#define	_PLOT_CL	0x000004
#define	_PLOT_CO	0x000008
#define	_PLOT_DT	0x000010
#define	_PLOT_ER	0x000020
#define	_PLOT_EM	0x000040
#define	_PLOT_LB	0x000200
#define	_PLOT_LM0	0x000400
#define	_PLOT_LM1	0x000800
#define	_PLOT_LM2	0x001000
#define	_PLOT_LM3	0x002000
#define	_PLOT_LM4	0x004000
#define	_PLOT_LM5	0x008000
#define	_PLOT_LM6	0x010000
#define	_PLOT_LM7	0x020000
#define	_PLOT_LN	0x040000
#define	_PLOT_MV	0x080000
#define	_PLOT_PT	0x100000
#define	_PLOT_RC	0x200000
#define	_PLOT_SP	0x400000

struct plotctx {
	FILE *fp;
	char *buf;

	char *matrix;
	long x, y, w, h;
	long sx, sy;	/* last sent coords (tek only currently) */
	long ox, dx, xnum, xdenom;
	long oy, dy, ynum, ydenom;
	int flags, caps, linemod;
	int (*putxy)(struct plotctx *, int, int, int);
	int (*puts)(struct plotctx *, const char *);
	int (*putfi)(struct plotctx *);

	const char *ci;
	const char *cl;
	const char *co;
	const char *dt;
	const char *er;
	const char *lb;
	const char *lm[8];
	const char *ln;
	const char *mv;
	const char *pt;
	const char *rc;
	const char *sp;
};

int _plot_asciixy(struct plotctx *, int, int, int);
int _plot_tekxy(struct plotctx *, int, int, int);
int _plot_plotxy(struct plotctx *, int, int, int);
int _plot_dumbxy(struct plotctx *, int, int, int);
int _plot_dumbfi(struct plotctx *);
int _plot_dumb_puts(struct plotctx *, const char *);
int _plot_out(struct plotctx *, const char *s, ...);
