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

/*							cacosf()
 *
 *	Complex circular arc cosine
 *
 *
 *
 * SYNOPSIS:
 *
 * void cacosf();
 * cmplxf z, w;
 *
 * cacosf( &z, &w );
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * w = arccos z  =  PI/2 - arcsin z.
 *
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10,+10     30000       9.2e-6       1.2e-6
 *
 */

#include <complex.h>
#include <math.h>

float complex
cacosf(float complex z)
{
	float complex w;

	w = casinf( z );
	w = (M_PI_2 - creal (w)) - cimag (w) * I;
	return (w);
}
