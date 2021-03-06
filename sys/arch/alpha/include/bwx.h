
/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ALPHA_BWX_H_
#define	_ALPHA_BWX_H_

/*
 * Alpha Byte/Word Extension instructions.
 *
 * These instructions are available on EV56 (21164A) and later processors.
 *
 * See "Alpha Architecture Handbook, Version 3", DEC order number EC-QD2KB-TE.
 */

#define BWX_EV56_INT8	(0L << 37)
#define BWX_EV56_INT4	(1L << 37)
#define BWX_EV56_INT2	(2L << 37)
#define BWX_EV56_INT1	(3L << 37)

static __inline u_int8_t
alpha_ldbu(__volatile u_int8_t *a0)
{
	u_int8_t v0;

	__asm __volatile("ldbu %0, %1"
		: "=r" (v0)
		: "m" (*a0));

	return (v0);
}

static __inline u_int16_t
alpha_ldwu(__volatile u_int16_t *a0)
{
	u_int16_t v0;

	__asm __volatile("ldwu %0, %1"
		: "=r" (v0)
		: "m" (*a0));

	return (v0);
}

static __inline u_int32_t
alpha_ldlu(__volatile u_int32_t *a0)
{
	return (*a0);
}

static __inline void
alpha_stb(__volatile u_int8_t *a0, u_int8_t a1)
{

	__asm __volatile("stb %1, %0"
		: "=m" (*a0)
		: "r" (a1)
		: "memory");
}

static __inline void
alpha_stw(__volatile u_int16_t *a0, u_int16_t a1)
{

	__asm __volatile("stw %1, %0"
		: "=m" (*a0)
		: "r" (a1)
		: "memory");
}

static __inline void
alpha_stl(__volatile u_int32_t *a0, u_int32_t a1)
{

	__asm __volatile("stl %1, %0"
		: "=m" (*a0)
		: "r" (a1)
		: "memory");
}

static __inline u_int8_t
alpha_sextb(u_int8_t a0)
{
	u_int8_t v0;

	__asm __volatile("sextb %1, %0"
		: "=r" (v0)
		: "r" (a0)
		: "memory");

	return (v0);
}

static __inline u_int16_t
alpha_sextw(u_int16_t a0)
{
	u_int16_t v0;

	__asm __volatile("sextw %1, %0"
		: "=r" (v0)
		: "r" (a0)
		: "memory");

	return (v0);
}

#endif /* _ALPHA_BWX_H_ */
