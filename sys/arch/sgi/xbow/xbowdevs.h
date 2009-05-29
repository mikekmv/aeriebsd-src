/*
 * THIS FILE AUTOMATICALLY GENERATED.  DO NOT EDIT.
 *
 *	ABSD: xbowdevs,v 1.2 2009/05/29 10:43:31 mickey Exp 
 */
/*
 * Copyright (c) 2008 Miodrag Vallat.
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
 *
 *
 * Copyright (c) 1995, 1996 Christopher G. Demetriou
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define	XBOW_VENDOR_SGI	0x0000
#define	XBOW_VENDOR_SGI2	0x0023
#define	XBOW_VENDOR_SGI3	0x0024
#define	XBOW_VENDOR_SGI4	0x0036
#define	XBOW_VENDOR_SGI5	0x02aa

/*
 * List of known products.  Grouped by ``manufacturer''.
 */

#define	XBOW_PRODUCT_SGI_XBOW	0x0000		/* XBow */
#define	XBOW_PRODUCT_SGI_XXBOW	0xd000		/* XXBow */

#define	XBOW_PRODUCT_SGI2_ODYSSEY	0xc013		/* Odyssey */

#define	XBOW_PRODUCT_SGI3_TPU	0xc202		/* TPU */
#define	XBOW_PRODUCT_SGI3_XBRIDGE	0xd002		/* XBridge */
/* PIC is really a single chip but two widgets, with 4 PCI-X slots per widget */
#define	XBOW_PRODUCT_SGI3_PIC0	0xd102		/* PIC (bus 0) */
#define	XBOW_PRODUCT_SGI3_PIC1	0xd112		/* PIC (bus 1) */
/* Supposedly a PIC-compatible chip, maybe a different revision */
/* product	SGI3	?		0xe000	? (bus 0) */
/* product	SGI3	?		0xe010	? (bus 1) */
#define	XBOW_PRODUCT_SGI3_TIOCA	0xe020		/* TIO:CA */

#define	XBOW_PRODUCT_SGI4_HEART	0xc001		/* Heart */
#define	XBOW_PRODUCT_SGI4_BRIDGE	0xc002		/* Bridge */
#define	XBOW_PRODUCT_SGI4_HUB	0xc101		/* Hub */
#define	XBOW_PRODUCT_SGI4_BEDROCK	0xc110		/* Bedrock */

#define	XBOW_PRODUCT_SGI5_IMPACT	0xc003		/* ImpactSR */
#define	XBOW_PRODUCT_SGI5_KONA	0xc102		/* Kona */
