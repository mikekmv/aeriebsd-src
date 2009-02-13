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

#ifndef lint
static const char     copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#ifndef lint
#if 0
static const char sccsid[] = "@(#)spline.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] = "$ABSD$";
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <err.h>

/*
 * XXX it should be possible to rewrite this to operate on a sliding window
 * of [x,y] of depath 3, imho, therefore no memory consuption at all,
 * maybe some recursion, just a little, doc, please ?.. (;
 */
struct proj {
	int	alloc, lbf, ubf;
	double	a, b, lb, ub, quant, mult, *val;
} x, y;

double	       *diag, *r;
double		dx = 1.;
double		ni = 100.;
int		n;
int		auta;
int		periodic;
double		konst = 0.0;
double		zero = 0.;

int		getfloat(double *p);
void		getlim(struct proj * p);

/* Spline fit technique
let x,y be vectors of abscissas and ordinates
    h	be vector of differences hi=xi-xi-1
    y"	be vector of 2nd derivs of approx function
If the points are numbered 0,1,2,...,n+1 then y" satisfies
(R W Hamming, Numerical Methods for Engineers and Scientists,
2nd Ed, p349ff)
	hiy"i-1+2(hi+hi+1)y"i+hi+1y"i+1
	
	= 6[(yi+1-yi)/hi+1-(yi-yi-1)/hi]   i=1,2,...,n

where y"0 = y"n+1 = 0
This is a symmetric tridiagonal system of the form

	| a1 h2		      |	 |y"1|	    |b1|
	| h2 a2 h3	      |	 |y"2|	    |b2|
	|    h3 a3 h4	      |	 |y"3|	=   |b3|
	|	  .	      |	 |  .|	    | .|
	|	     .	      |	 |  .|	    | .|
It can be triangularized into
	| d1 h2		      |	 |y"1|	    |r1|
	|    d2 h3	      |	 |y"2|	    |r2|
	|	d3 h4	      |	 |y"3|	=   |r3|
	|	   .	      |	 |  .|	    | .|
	|	      .	      |	 |  .|	    | .|
where
	d1 = a1

	r0 = 0

	di = ai - hi2/di-1	1<i<_n

	ri = bi - hiri-1/di-1i	1<_i<_n

the back solution is
	y"n = rn/dn

	y"i = (ri-hi+1y"i+1)/di 1<_i<n

superficially, di and ri don't have to be stored for they can be
recalculated backward by the formulas

	di-1 = hi2/(ai-di)	1<i<_n

	ri-1 = (bi-ri)di-1/hi	1<i<_n

unhappily it turns out that the recursion forward for d
is quite strongly geometrically convergent--and is wildly
unstable going backward.
There's similar trouble with r, so the intermediate
results must be kept.

Note that n-1 in the program below plays the role of n+1 in the theory

Other boundary conditions_________________________

The boundary conditions are easily generalized to handle

	y0" = ky1", yn+1"   = kyn"

for some constant k.  The above analysis was for k = 0;
k = 1 fits parabolas perfectly as well as stright lines;
k = 1/2 has been recommended as somehow pleasant.

All that is necessary is to add h1 to a1 and hn+1 to an.


Periodic case_____________

To do this, add 1 more row and column thus

	| a1 h2		   h1 |	 |y1"|	   |b1|
	| h2 a2 h3	      |	 |y2"|	   |b2|
	|    h3 a4 h4	      |	 |y3"|	   |b3|
	|		      |	 |  .|  =  | .|
	|	      .	      |	 |  .|	   | .|
	| h1		h0 a0 |	 |  .|	   | .|

where h0=_ hn+1

The same diagonalization procedure works, except for
the effect of the 2 corner elements.  Let si be the part
of the last element in the ith "diagonalized" row that
arises from the extra top corner element.

		s1 = h1

		si = -si-1hi/di-1	2<_i<_n+1

After "diagonalizing", the lower corner element remains.
Call ti the bottom element that appears in the ith colomn
as the bottom element to its left is eliminated

		t1 = h1

		ti = -ti-1hi/di-1

Evidently ti = si.
Elimination along the bottom row
introduces further corrections to the bottom right element
and to the last element of the right hand side.
Call these corrections u and v.

	u1 = v1 = 0

	ui = ui-1-si-1*ti-1/di-1

	vi = vi-1-ri-1*ti-1/di-1	2<_i<_n+1

The back solution is now obtained as follows

	y"n+1 = (rn+1+vn+1)/(dn+1+sn+1+tn+1+un+1)

	y"i = (ri-hi+1*yi+1-si*yn+1)/di 1<_i<_n

Interpolation in the interval xi<_x<_xi+1 is by the formula

	y = yix+ + yi+1x- -(h2i+1/6)[y"i(x+-x+3)+y"i+1(x--x-3)]
where
	x+ = xi+1-x

	x- = x-xi
*/

double
rhs(int i)
{
	int		i_;
	double		zz;
	i_ = i == n - 1 ? 0 : i;
	zz = (y.val[i] - y.val[i - 1]) / (x.val[i] - x.val[i - 1]);
	return (6 * ((y.val[i_ + 1] - y.val[i_]) / (x.val[i + 1] - x.val[i]) - zz));
}

int
spline()
{
	double		d, s, u, v, hi, hi1;
	double		h;
	double		D2yi, D2yi1, D2yn1, x0, x1, yy, a;
	int		end;
	double		corr;
	int		i, j, m;
	if (n < 3)
		return (0);
	if (periodic)
		konst = 0;
	d = 1;
	r[0] = 0;
	s = periodic ? -1 : 0;
	u = v = zero;
	for (i = 0; ++i < n - !periodic;) {	/* triangularize */
		hi = x.val[i] - x.val[i - 1];
		hi1 = i == n - 1 ? x.val[1] - x.val[0] :
			x.val[i + 1] - x.val[i];
		if (hi1 * hi <= 0)
			return (0);
		if (i != 1) {
			u -= s * s / d;
			v -= s * r[i - 1] / d;
		}
		r[i] = rhs(i) - hi * r[i - 1] / d;
		s = -hi * s / d;
		a = 2 * (hi + hi1);
		if (i == 1)
			a += konst * hi;
		if (i == n - 2)
			a += konst * hi1;
		diag[i] = d = i == 1 ? a :
			a - hi * hi / d;
	}
	D2yi = D2yn1 = 0;
	for (i = n - !periodic; --i >= 0;) {	/* back substitute */
		end = i == n - 1;
		hi1 = end ? x.val[1] - x.val[0] :
			x.val[i + 1] - x.val[i];
		D2yi1 = D2yi;
		if (i > 0) {
			hi = x.val[i] - x.val[i - 1];
			corr = end ? 2 * s + u : zero;
			D2yi = (end * v + r[i] - hi1 * D2yi1 - s * D2yn1) /
				(diag[i] + corr);
			if (end)
				D2yn1 = D2yi;
			if (i > 1) {
				a = 2 * (hi + hi1);
				if (i == 1)
					a += konst * hi;
				if (i == n - 2)
					a += konst * hi1;
				d = diag[i - 1];
				s = -s * d / hi;
			}
		} else
			D2yi = D2yn1;
		if (!periodic) {
			if (i == 0)
				D2yi = konst * D2yi1;
			if (i == n - 2)
				D2yi1 = konst * D2yi;
		}
		if (end)
			continue;
		m = hi1 > 0 ? ni : -ni;
		m = 1.001 * m * hi1 / (x.ub - x.lb);
		if (m <= 0)
			m = 1;
		h = hi1 / m;
		for (j = m; j > 0 || (i == 0 && j == 0); j--) { /* interpolate */
			x0 = (m - j) * h / hi1;
			x1 = j * h / hi1;
			yy = D2yi * (x0 - x0 * x0 * x0) +
			    D2yi1 * (x1 - x1 * x1 * x1);
			yy = y.val[i] * x0 + y.val[i + 1] * x1 -
			    hi1 * hi1 * yy / 6;
			printf("%f %f\n", x.val[i] + j * h, yy);
		}
	}

	return (1);
}

int
approximate(void)
{
	return 0;
}

void
readin()
{
	float xi, yi;
	int i;

	x.val = NULL;
	y.val = NULL;

	for (i = 0; scanf("%f %f\n", &xi, &yi) == 2; i++) {
		if (i == x.alloc) {
			if (!(x.val = realloc(x.val, sizeof(x.val[0]) *
			    (x.alloc += 1000))))
				err(1, "realloc");
			if (!(y.val = realloc(y.val, sizeof(y.val[0]) *
			    (y.alloc += 1000))))
				err(1, "realloc");
		}

		if (auta)
			x.val[i] = i * dx + x.lb;
		else  {
			x.val[i] = xi;
			y.val[i] = yi;
		}
	}

	n = i;
}

void
getlim(p)
	struct proj    *p;
{
	register int	i;

	if (!p->lbf) {
		p->lb = p->val[0];
		for (i = 1; i < n; i++)
			if (p->lb > p->val[i])
				p->lb = p->val[i];
	}
	if (!p->ubf) {
		p->ub = p->val[0];
		for (i = 1; i < n; i++)
			if (p->ub < p->val[i])
				p->ub = p->val[i];
	}
}

__dead void
usage(void)
{
	extern char *__progname;
	fprintf(stderr, "usage: %s [-aknpx]\n", __progname);
	exit(1);
}

void
sqfilter(void)
{
	double d, dy, avg;
	int i;

	for (d = 0, avg = 0, i = 0; i < n; i++) {
		d += y.val[i] * y.val[i];
		avg += y.val[i];
	}

	avg /= n;
	d = sqrt(d / n - avg * avg);
	for (i = 1; i < n; i++) {
		dy = y.val[i] - y.val[i - 1];
		if (fabs(dy) > d)
			y.val[i] = y.val[i - 1] + (dy < 0? -d : d);
	}
}

int
main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	approx, filter;
	int	i, ch;

	x.lbf = x.ubf = y.lbf = y.ubf = 0;
	x.lb = x.ub = y.lb = y.ub = 0;

	approx = filter = 0;
	while ((ch = getopt(argc, argv, "a:cCfk:n:px:")) != -1)
		switch (ch) {
		case 'a':
			auta = 1;
			dx = atof(optarg);
			break;
		case 'c':
			approx = 1;
			break;
		case 'f':
			filter = 1;
			break;
		case 'k':
			konst = atof(optarg);
			break;
		case 'n':
			ni = atof(optarg);
			break;
		case 'p':
			periodic = 1;
			break;
		case 'x':
			x.lb = atof(optarg);
			x.lbf = 1;
			if (argv[optind]) {
				x.ub = atof(argv[optind++]);
				x.ubf = 1;
			}
			break;
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	/* accept file name in the cmd line in the future? */
	if (argc)
		usage();

	readin();
	if (filter)
		sqfilter();
	getlim(&x);
	getlim(&y);
	i = (n + 1) * sizeof(dx);
	diag = (double *) malloc(i);
	if (diag == NULL)
		err(1, "diag");
	r = (double *) malloc(i);
	if (r == NULL)
		err(1, "r");
	if (!(!approx && spline()) &&
	    !(approx && approximate()))
		for (i = 0; i < n; i++)
			printf("%f %f\n", x.val[i], y.val[i]);
	exit(0);
}
