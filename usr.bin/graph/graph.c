/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (C) Caldera International Inc.  2001-2002.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code and documentation must retain the above
 *    copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed or owned by Caldera
 *	International, Inc.
 * 4. Neither the name of Caldera International, Inc. nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR ANY DIRECT,
 * INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#ifndef lint
#if 0
static const char sccsid[] = "@(#)graph.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] =
    "$ABSD: graph.c,v 1.2 2009/05/28 13:59:04 mickey Exp $";
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <err.h>
#include <plot.h>

#define F	.25

struct xy {
	int		xlbf;	/* flag:explicit lower bound */
	int		xubf;	/* flag:explicit upper bound */
	int		xqf;	/* flag:explicit quantum */
	double		(*xf) ();	/* transform function, e.g. log */
	float		xa, xb; /* scaling coefficients */
	float		xlb, xub;	/* lower and upper bound */
	float		xquant; /* quantum */
	float		xoff;	/* screen offset fraction */
	float		xsize;	/* screen fraction */
	int		xbot, xtop;	/* screen coords of border */
	float		xmult;	/* scaling constant */
} xd, yd;
struct val {
	float		xv;
	float		yv;
	int		lblptr;
} *xx;

char	       *labels;
int		labsiz;

int		tick = 50;
int		top = 4000;
int		bot = 200;
float		absbot;
int		n;
int		erasf = 1;
int		gridf = 2;
int		symbf = 0;
int		absf = 0;
int		transf;
int		brkf;
float		dx;
char	       *plotsymb, *titlep;

#define BSIZ 80
char		labbuf[BSIZ];

int getstring(void);
void axlab(void *, char, struct xy *);
void usage(void);
void plot(void *);
void axes(void *);
void title(void *);
void transpose(void);
void readin(void);
int  copystring(int);
void submark(int *, int *, float, struct xy *);
void scale( struct xy *, struct val *);
void init(struct xy *);
int symbol(void *, int, int, int);
int conv(float, struct xy *, int *);
int setmark(int *, struct xy *);

char	*modes[] = {
	"disconnected",
	"solid",
	"dotted",
	"dotdashed",
	"shortdashed",
	"longdashed"
};
int		mode = 1;

double
ident(double x)
{
	return (x);
}

int
main(argc, argv)
	int	argc;
	char	*argv[];
{
	struct xy *p;
	void	*pl;
	char	*term = NULL;
	int	ch;

	init(&xd);
	init(&yd);
	xd.xsize = yd.xsize = 1.;
	xx = (struct val *) malloc((unsigned) sizeof(struct val));
	labels = malloc(1);
	labels[labsiz++] = 0;

	xd.xlb = yd.xlb = 0;
	xd.xub = yd.xub = 0;
	while ((ch = getopt(argc, argv, "a:bc:d:g:h:l:m:r:stT:u:w:x:y:")) != -1)
		switch (ch) {
		case 'T':	/* use this terminal */
			term = optarg;
			break;

		case 'l':	/* label for plot */
			titlep = optarg;
			break;

		case 'd':	/* disconnected,obsolete option */
		case 'm':	/* line mode */
			mode = atoi(optarg);
			if (mode >= sizeof(modes) / sizeof(*modes))
				mode = 1;
			else if (mode < 0)
				mode = 0;
			break;

		case 'a':	/* automatic abscissas */
			absf = 1;
			dx = atof(optarg);	/* defaults to 1 */
			if (argv[optind])
				absbot = atof(argv[optind++]);	/* if fails absf = 2 */
			break;

		case 's':	/* save screen, overlay plot */
			erasf = 0;
			break;

		case 'g':	/* grid style 0 none, 1 ticks, 2 full */
			gridf = atoi(optarg);
			/* temp = argv[0][1] - '0';*/	/* for caompatibility */
			if (gridf < 0 && gridf > 2)
				gridf = 0;
			break;

		case 'c':	/* character(s) for plotting */
			symbf = 1;
			plotsymb = optarg;
			break;
		case 't':	/* transpose */
			transf = 1;
			break;
		case 'b':	/* breaks */
			brkf = 1;
			break;
		case 'x':	/* x limits */
		case 'y':
			p = &xd;
			if (ch == 'y')
				p = &yd;
			if (optarg[0] == 'l') {
				p->xf = log10;
				optarg = argv[optind++];
			}
			if (optarg) {
				p->xlb = atof(optarg);
				p->xlbf = 1;
				optarg = argv[optind++];
			}
			if (optarg) {
				p->xub = atof(optarg);
				p->xubf = 1;
				optarg = argv[optind++];
			}
			if (optarg) {
				p->xquant = atof(optarg);
				p->xqf = 1;
				optarg = argv[optind++];
			}
			break;
		case 'h':	/* set height of plot */
			yd.xsize = atof(optarg);
			break;
		case 'w':	/* set width of plot */
			xd.xsize = atof(optarg);
			break;
		case 'r':	/* set offset to right */
			xd.xoff = atof(optarg);
			break;
		case 'u':	/* set offset up the screen */
			yd.xoff = atof(optarg);
			break;
		default:
			usage();
		}

	if (!(pl = plot_open(term, "plot", stdout)))
		errx(1, "cannot open a plot");

	plot_space(pl, 0, 0, 4096, 4096);
	if (erasf)
		plot_erase(pl);

	readin();
	transpose();
	scale(&xd, (struct val *) & xx->xv);
	scale(&yd, (struct val *) & xx->yv);
	axes(pl);
	title(pl);
	plot(pl);
	plot_move(pl, 1, 1);
	plot_close(pl);
	return (0);
}

void
init(struct xy *p)
{
	p->xf = ident;
	p->xmult = 1;
}

void
readin(void)
{
	int		t;
	struct val     *temp;

	if (absf == 1) {
		if (xd.xlbf)
			absbot = xd.xlb;
		else if (xd.xf == log10)
			absbot = 1;
	}
	for (;;) {
		temp = (struct val *) realloc((char *) xx,
				   (unsigned) (n + 1) * sizeof(struct val));
		if (temp == 0)
			return;
		xx = temp;
		if (absf)
			xx[n].xv = n * dx + absbot;
		else if (scanf("%f %f", &xx[n].xv, &xx[n].yv) != 2)
			return;
		xx[n].lblptr = -1;
		t = getstring();
		if (t > 0)
			xx[n].lblptr = copystring(t);
		n++;
		if (t < 0)
			return;
	}
}

void
transpose(void)
{
	int		i;
	float		f;
	struct xy	t;
	if (!transf)
		return;
	t = xd;
	xd = yd;
	yd = t;
	for (i = 0; i < n; i++) {
		f = xx[i].xv;
		xx[i].xv = xx[i].yv;
		xx[i].yv = f;
	}
}

int
copystring(int k)
{
	register char  *temp;
	int		i;
	int		q;

	temp = realloc(labels, (unsigned) (labsiz + 1 + k));
	if (temp == 0)
		return (0);
	labels = temp;
	q = labsiz;
	for (i = 0; i <= k; i++)
		labels[labsiz++] = labbuf[i];
	return (q);
}

float
modceil(float f, float t)
{
	t = fabs(t);
	return (ceil(f / t) * t);
}

float
modfloor(float f, float t)
{
	t = fabs(t);
	return (floor(f / t) * t);
}

/*
 * Compute upper and lower bounds for the given descriptor.
 * We may already have one or both.  We assume that if n==0,
 * v[0].xv is a valid limit value.
 */
void
getlim(struct xy *p, struct val *v)
{
	int	i;

	if (!p->xlbf) {		/* need lower bound */
		p->xlb = v[0].xv;
		for (i = 1; i < n; i++)
			if (p->xlb > v[i].xv)
				p->xlb = v[i].xv;
	}
	if (!p->xubf) {		/* need upper bound */
		p->xub = v[0].xv;
		for (i = 1; i < n; i++)
			if (p->xub < v[i].xv)
				p->xub = v[i].xv;
	}
}

struct z {
	float		lb, ub, mult, quant;
}		setloglim(), setlinlim();

void
setlim(struct xy *p)
{
	float		t, delta, sign;
	struct z	z;
	int		mark[50];
	float		lb, ub;
	int		lbf, ubf;

	lb = p->xlb;
	ub = p->xub;
	delta = ub - lb;
	if (p->xqf) {
		if (delta * p->xquant <= 0)
			usage();
		return;
	}
	sign = 1;
	lbf = p->xlbf;
	ubf = p->xubf;
	if (delta < 0) {
		sign = -1;
		t = lb;
		lb = ub;
		ub = t;
		t = lbf;
		lbf = ubf;
		ubf = t;
	} else if (delta == 0) {
		if (ub > 0) {
			ub = 2 * ub;
			lb = 0;
		} else if (lb < 0) {
			lb = 2 * lb;
			ub = 0;
		} else {
			ub = 1;
			lb = -1;
		}
	}
	if (p->xf == log10 && lb > 0 && ub > lb) {
		z = setloglim(lbf, ubf, lb, ub);
		p->xlb = z.lb;
		p->xub = z.ub;
		p->xmult *= z.mult;
		p->xquant = z.quant;
		if (setmark(mark, p) < 2) {
			p->xqf = lbf = ubf = 1;
			lb = z.lb;
			ub = z.ub;
		} else
			return;
	}
	z = setlinlim(lbf, ubf, lb, ub);
	if (sign > 0) {
		p->xlb = z.lb;
		p->xub = z.ub;
	} else {
		p->xlb = z.ub;
		p->xub = z.lb;
	}
	p->xmult *= z.mult;
	p->xquant = sign * z.quant;
}

struct z
setloglim(int lbf, int ubf, float lb, float ub)
{
	float		r, s, t;
	struct z	z;

	for (s = 1; lb * s < 1; s *= 10);
	lb *= s;
	ub *= s;
	for (r = 1; 10 * r <= lb; r *= 10);
	for (t = 1; t < ub; t *= 10);
	z.lb = !lbf ? r : lb;
	z.ub = !ubf ? t : ub;
	if (ub / lb < 100) {
		if (!lbf) {
			if (lb >= 5 * z.lb)
				z.lb *= 5;
			else if (lb >= 2 * z.lb)
				z.lb *= 2;
		}
		if (!ubf) {
			if (ub * 5 <= z.ub)
				z.ub /= 5;
			else if (ub * 2 <= z.ub)
				z.ub /= 2;
		}
	}
	z.mult = s;
	z.quant = r;
	return (z);
}

struct z
setlinlim(int lbf, int ubf, float xlb, float xub)
{
	struct z	z;
	float		r, s, delta;
	float		ub, lb;

loop:
	ub = xub;
	lb = xlb;
	delta = ub - lb;
	/* scale up by s, a power of 10, so range (delta) exceeds 1 */
	/* find power of 10 quantum, r, such that delta/10<=r<delta */
	r = s = 1;
	while (delta * s < 10)
		s *= 10;
	delta *= s;
	while (10 * r < delta)
		r *= 10;
	lb *= s;
	ub *= s;
	/* set r=(1,2,5)*10**n so that 3-5 quanta cover range */
	if (r >= delta / 2)
		r /= 2;
	else if (r < delta / 5)
		r *= 2;
	z.ub = ubf ? ub : modceil(ub, r);
	z.lb = lbf ? lb : modfloor(lb, r);
	if (!lbf && z.lb <= r && z.lb > 0) {
		xlb = 0;
		goto loop;
	} else if (!ubf && z.ub >= -r && z.ub < 0) {
		xub = 0;
		goto loop;
	}
	z.quant = r;
	z.mult = s;
	return (z);
}

void
scale(p, v)
	struct xy	*p;
	struct val     *v;
{
	float		edge;

	getlim(p, v);
	setlim(p);
	edge = top - bot;
	p->xa = p->xsize * edge / ((*p->xf) (p->xub) - (*p->xf) (p->xlb));
	p->xbot = bot + edge * p->xoff;
	p->xtop = p->xbot + (top - bot) * p->xsize;
	p->xb = p->xbot - (*p->xf) (p->xlb) * p->xa + .5;
}

void
axes(void *pl)
{
	int		i;
	int		mark[50];
	int		xn, yn;

	if (gridf == 0)
		return;

	plot_line(pl, xd.xbot, yd.xbot, xd.xtop, yd.xbot);
	plot_cont(pl, xd.xtop, yd.xtop);
	plot_cont(pl, xd.xbot, yd.xtop);
	plot_cont(pl, xd.xbot, yd.xbot);

	xn = setmark(mark, &xd);
	for (i = 0; i < xn; i++) {
		if (gridf == 2)
			plot_line(pl, mark[i], yd.xbot, mark[i], yd.xtop);
		if (gridf == 1) {
			plot_line(pl, mark[i], yd.xbot, mark[i], yd.xbot + tick);
			plot_line(pl, mark[i], yd.xtop - tick, mark[i], yd.xtop);
		}
	}
	yn = setmark(mark, &yd);
	for (i = 0; i < yn; i++) {
		if (gridf == 2)
			plot_line(pl, xd.xbot, mark[i], xd.xtop, mark[i]);
		if (gridf == 1) {
			plot_line(pl, xd.xbot, mark[i], xd.xbot + tick, mark[i]);
			plot_line(pl, xd.xtop - tick, mark[i], xd.xtop, mark[i]);
		}
	}
}

int
setmark(xmark, p)
	int	       *xmark;
	struct xy	*p;
{
	int		xn = 0;
	float		x, xl, xu;
	float		q;
	if (p->xf == log10 && !p->xqf) {
		for (x = p->xquant; x < p->xub; x *= 10) {
			submark(xmark, &xn, x, p);
			if (p->xub / p->xlb <= 100) {
				submark(xmark, &xn, 2 * x, p);
				submark(xmark, &xn, 5 * x, p);
			}
		}
	} else {
		xn = 0;
		q = p->xquant;
		if (q > 0) {
			xl = modceil(p->xlb + q / 6, q);
			xu = modfloor(p->xub - q / 6, q) + q / 2;
		} else {
			xl = modceil(p->xub - q / 6, q);
			xu = modfloor(p->xlb + q / 6, q) - q / 2;
		}
		for (x = xl; x <= xu; x += fabs(p->xquant))
			xmark[xn++] = (*p->xf) (x) * p->xa + p->xb;
	}
	return (xn);
}

void
submark(int *xmark, int *pxn, float x, struct xy *p)
{
	if (1.001 * p->xlb < x && .999 * p->xub > x)
		xmark[(*pxn)++] = log10(x) * p->xa + p->xb;
}

void
plot(void *pl)
{
	int		ix, iy;
	int		i;
	int		conn;

	conn = 0;
	if (mode != 0)
		plot_linemod(pl, mode);
	for (i = 0; i < n; i++) {
		if (!conv(xx[i].xv, &xd, &ix) ||
		    !conv(xx[i].yv, &yd, &iy)) {
			conn = 0;
			continue;
		}
		if (mode != 0) {
			if (conn != 0)
				plot_cont(pl, ix, iy);
			else
				plot_move(pl, ix, iy);
			conn = 1;
		}
		conn &= symbol(pl, ix, iy, xx[i].lblptr);
	}
	plot_linemod(pl, 1);
}

int
conv(float xv, struct xy *p, int *ip)
{
	long		ix;
	ix = p->xa * (*p->xf) (xv * p->xmult) + p->xb;
	if (ix < p->xbot || ix > p->xtop)
		return (0);
	*ip = ix;
	return (1);
}

int
getstring(void)
{
	int		i;
	char		junk[20];
	i = scanf("%1s", labbuf);
	if (i == -1)
		return (-1);
	switch (*labbuf) {
	default:
		if (!isdigit(*labbuf)) {
			ungetc(*labbuf, stdin);
			i = scanf("%s", labbuf);
			break;
		}
	case '.':
	case '+':
	case '-':
		ungetc(*labbuf, stdin);
		return (0);
	case '"':
		i = scanf("%[^\"\n]", labbuf);
		scanf("%[\"]", junk);
		break;
	}
	if (i == -1)
		return (-1);
	return (strlen(labbuf));
}

int
symbol(void *pl, int ix, int iy, int k)
{

	if (symbf == 0 && k < 0) {
		if (mode == 0)
			plot_point(pl, ix, iy);
		return (1);
	} else {
		plot_move(pl, ix, iy);
		plot_label(pl, k >= 0 ? labels + k : plotsymb);
		plot_move(pl, ix, iy);
		return (!brkf || k < 0);
	}
}

void
title(void *pl)
{
	plot_move(pl, xd.xbot, yd.xbot - 60);
	if (titlep) {
		plot_label(pl, titlep);
		plot_label(pl, "	      ");
	}
	if (erasf && gridf) {
		axlab(pl, 'x', &xd);
		plot_label(pl, "	 ");
		axlab(pl, 'y', &yd);
	}
}

void
axlab(void *pl, char c, struct xy *p)
{
	char	buf[50];
	snprintf(buf, sizeof(buf), "%g -%s%c- %g", p->xlb / p->xmult,
	    p->xf == log10 ? "log " : "", c, p->xub / p->xmult);
	plot_label(pl, buf);
}

void
usage()
{
	extern char *__progname;
	fprintf(stderr, "usage: %s \n", __progname);
	exit(1);
}
