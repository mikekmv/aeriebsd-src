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


/* Descriptions of known devices. */
struct xbow_product {
	uint32_t vendor;
	uint32_t product;
	const char *productname;
};

static const struct xbow_product xbow_products[] = {
	{
	    XBOW_VENDOR_SGI, XBOW_PRODUCT_SGI_XBOW,
	    "XBow",
	},
	{
	    XBOW_VENDOR_SGI, XBOW_PRODUCT_SGI_XXBOW,
	    "XXBow",
	},
	{
	    XBOW_VENDOR_SGI2, XBOW_PRODUCT_SGI2_ODYSSEY,
	    "Odyssey",
	},
	{
	    XBOW_VENDOR_SGI3, XBOW_PRODUCT_SGI3_TPU,
	    "TPU",
	},
	{
	    XBOW_VENDOR_SGI3, XBOW_PRODUCT_SGI3_XBRIDGE,
	    "XBridge",
	},
	{
	    XBOW_VENDOR_SGI3, XBOW_PRODUCT_SGI3_PIC0,
	    "PIC (bus 0)",
	},
	{
	    XBOW_VENDOR_SGI3, XBOW_PRODUCT_SGI3_PIC1,
	    "PIC (bus 1)",
	},
	{
	    XBOW_VENDOR_SGI3, XBOW_PRODUCT_SGI3_TIOCA,
	    "TIO:CA",
	},
	{
	    XBOW_VENDOR_SGI4, XBOW_PRODUCT_SGI4_HEART,
	    "Heart",
	},
	{
	    XBOW_VENDOR_SGI4, XBOW_PRODUCT_SGI4_BRIDGE,
	    "Bridge",
	},
	{
	    XBOW_VENDOR_SGI4, XBOW_PRODUCT_SGI4_HUB,
	    "Hub",
	},
	{
	    XBOW_VENDOR_SGI4, XBOW_PRODUCT_SGI4_BEDROCK,
	    "Bedrock",
	},
	{
	    XBOW_VENDOR_SGI5, XBOW_PRODUCT_SGI5_IMPACT,
	    "ImpactSR",
	},
	{
	    XBOW_VENDOR_SGI5, XBOW_PRODUCT_SGI5_KONA,
	    "Kona",
	},
	{ 0, 0, NULL, }
};

