/*
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if defined(LIBM_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

/*							csinf()
 *
 *	Complex circular sine
 *
 *
 *
 * SYNOPSIS:
 *
 * void csinf();
 * cmplxf z, w;
 *
 * csinf( &z, &w );
 *
 *
 *
 * DESCRIPTION:
 *
 * If
 *     z = x + iy,
 *
 * then
 *
 *     w = sin x  cosh y  +  i cos x sinh y.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10,+10     30000       1.9e-7      5.5e-8
 *
 */

#include <complex.h>
#include <math.h>

/* calculate cosh and sinh */

static void
cchshf(float xx, float *c, float *s)
{
	float x, e, ei;

	x = xx;
	if(fabsf(x) <= 0.5f) {
		*c = coshf(x);
		*s = sinhf(x);
	}
	else {
		e = expf(x);
		ei = 0.5f/e;
		e = 0.5f * e;
		*s = e - ei;
		*c = e + ei;
	}
}

float complex
csinf(float complex z)
{
	float complex w;
	float ch, sh;

	cchshf((float) cimag(z), &ch, &sh);
	w = sinf(creal(z)) * ch  + (cosf(creal(z)) * sh) * I;
	return (w);
}
