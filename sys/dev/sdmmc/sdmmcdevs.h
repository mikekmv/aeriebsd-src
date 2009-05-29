/*
 * THIS FILE AUTOMATICALLY GENERATED.  DO NOT EDIT.
 *
 *	ABSD: sdmmcdevs,v 1.2 2009/05/29 10:43:32 mickey Exp 
 */
/*
 * Copyright (c) 2009 Michael Shalayeff
 * Copyright (c) 2006 Uwe Stuehler <uwe@openbsd.org>
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
 * Copyright (c) 1998, Christos Zoulas
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
 *      This product includes software developed by Christos Zoulas
 * 4. The name of the author(s) may not be used to endorse or promote products
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
 */
/*
 * List of known SD card vendors
 */

#define	SDMMC_VENDOR_CGUYS	0x0092	/* C-guys, Inc. */
#define	SDMMC_VENDOR_TOSHIBA	0x0098	/* Toshiba */
#define	SDMMC_VENDOR_SOCKETCOM	0x0104	/* Socket Communications, Inc. */
#define	SDMMC_VENDOR_ATHEROS	0x0271	/* Atheros */
#define	SDMMC_VENDOR_SYCHIP	0x02db	/* SyChip Inc. */
#define	SDMMC_VENDOR_SPECTEC	0x02fe	/* Spectec Computer Co., Ltd */
#define	SDMMC_VENDOR_GLOBALSAT	0x0501	/* Globalsat Technology Co. */
#define	SDMMC_VENDOR_MEDIATEK	0x037a	/* MediaTek Inc. */
#define	SDMMC_VENDOR_ABOCOM	0x13d1	/* AboCom Systems, Inc. */

/*
 * List of known products, grouped by vendor
 */

/* AboCom Systems, Inc. */
#define	SDMMC_CIS_ABOCOM_SDW11G	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_ABOCOM_SDW11G	0xac02

/* Atheros */
#define	SDMMC_CIS_ATHEROS_AR6001_8	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_ATHEROS_AR6001_8	0x0108
#define	SDMMC_CIS_ATHEROS_AR6001_9	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_ATHEROS_AR6001_9	0x0109
#define	SDMMC_CIS_ATHEROS_AR6001_a	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_ATHEROS_AR6001_a	0x010a
#define	SDMMC_CIS_ATHEROS_AR6001_b	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_ATHEROS_AR6001_b	0x010b

/* C-guys, Inc. */
#define	SDMMC_CIS_CGUYS_TIACX100	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_CGUYS_TIACX100	0x0001
#define	SDMMC_CIS_CGUYS_SDFMRADIO2	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_CGUYS_SDFMRADIO2	0x0005
#define	SDMMC_CIS_CGUYS_SDFMRADIO	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_CGUYS_SDFMRADIO	0x5544

/* Globalsat Technology Co. */
#define	SDMMC_CIS_GLOBALSAT_SD501	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_GLOBALSAT_SD501	0xf501

/* MediaTek Inc. */
#define	SDMMC_CIS_MEDIATEK_S2YWLAN	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_MEDIATEK_S2YWLAN	0x5911

/* Spectec Computer Co., Ltd */
#define	SDMMC_CIS_SPECTEC_SDW820	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_SPECTEC_SDW820	0x2128

/* SyChip Inc. */
#define	SDMMC_CIS_SYCHIP_WLAN6060SD	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_SYCHIP_WLAN6060SD	0x0002

/* Toshiba */
#define	SDMMC_CIS_TOSHIBA_SDBTCARD1	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_TOSHIBA_SDBTCARD1	0x0001
#define	SDMMC_CIS_TOSHIBA_SDBTCARD2	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_TOSHIBA_SDBTCARD2	0x0002
#define	SDMMC_CIS_TOSHIBA_SDBTCARD3	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_TOSHIBA_SDBTCARD3	0x0003

/* Socket Communications, Inc. */
#define	SDMMC_CIS_SOCKETCOM_SDSCANNER	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_SOCKETCOM_SDSCANNER	0x005e
#define	SDMMC_CIS_SOCKETCOM_BTCARD	{ NULL, NULL, NULL, NULL }
#define	SDMMC_PRODUCT_SOCKETCOM_BTCARD	0x00c5
