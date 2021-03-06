/*
 * Copyright (c) 1996 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <machine/asm.h>

#if defined(LIBC_SCCS)
	.section	.rodata
	.asciz "$ABSD: byte_swap_2.S,v 1.2 2009/05/23 01:24:03 mickey Exp $"
	.text
#endif

#ifndef NAME
#define	NAME byte_swap_2
#endif

/*
 * Byte-swap a 2-byte quantity.  (Convert 0x0123 to 0x2301.)
 *
 * Argument is an unsigned 2-byte integer (u_int16_t).
 */
LEAF(NAME, 1)			/* a0 contains 0x0123 */
	extbl	a0, 0, t0	/* t0 = 0x  23 */
	extbl	a0, 1, t1	/* t1 = 0x  01 */
	sll	t0, 8, t0	/* t1 = 0x23   */
	or	t0, t1, v0	/* v0 = 0x2301 */
	RET
END(NAME)
