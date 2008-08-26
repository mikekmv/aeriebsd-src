
/*-
 * Copyright (c) 1996, 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _HP300_INTR_H_
#define	_HP300_INTR_H_

#include <machine/psl.h>
#include <sys/evcount.h>
#include <sys/queue.h>

#ifdef _KERNEL
struct isr {
	LIST_ENTRY(isr) isr_link;
	int		(*isr_func)(void *);
	void		*isr_arg;
	int		isr_ipl;
	int		isr_priority;
	struct evcount	isr_count;
};

#define NISR		8

/*
 * Interrupt "levels".  These are a more abstract representation
 * of interrupt levels, and do not have the same meaning as m68k
 * CPU interrupt levels.  They serve two purposes:
 *
 *	- properly order ISRs in the list for that CPU ipl
 *	- compute CPU PSL values for the spl*() calls.
 */
#define	IPL_NONE	0
#define	IPL_SOFTNET	1
#define	IPL_SOFTCLOCK	1
#define	IPL_BIO		2
#define	IPL_NET		3
#define	IPL_TTY		4
#define	IPL_VM		5
#define	IPL_CLOCK	6
#define	IPL_STATCLOCK	6
#define	IPL_HIGH	7

/*
 * This array contains the appropriate PSL_S|PSL_IPL? values
 * to raise interrupt priority to the requested level.
 */
extern	unsigned short hp300_varpsl[NISR];

/* These spl calls are used by machine-independent code. */
#define	splsoft()		_splraise(PSL_S | PSL_IPL1)
#define	splsoftclock()		splsoft()
#define	splsoftnet()		splsoft()
#define	splbio()		_splraise(hp300_varpsl[IPL_BIO])
#define	splnet()		_splraise(hp300_varpsl[IPL_NET])
#define	spltty()		_splraise(hp300_varpsl[IPL_TTY])
#define	splclock()		_splraise(PSL_S | PSL_IPL6)
#define	splstatclock()		_splraise(PSL_S | PSL_IPL6)
#define	splvm()			_splraise(hp300_varpsl[IPL_VM])
#define	splhigh()		_spl(PSL_S | PSL_IPL7)
#define	splsched()		splhigh()

/* watch out for side effects */
#define	splx(s)			((s) & PSL_IPL ? _spl((s)) : spl0())

/*
 * Simulated software interrupt register.
 */
extern volatile u_int8_t ssir;

#define	SIR_NET		0x01
#define	SIR_CLOCK	0x02

#define	siron(mask)	\
	__asm __volatile ( "orb %1,%0" : "=m" (ssir) : "i" (mask))
#define	siroff(mask)	\
	__asm __volatile ( "andb %1,%0" : "=m" (ssir) : "ir" (~(mask)))

#define	setsoftnet()	siron(SIR_NET)
#define	setsoftclock()	siron(SIR_CLOCK)

/* locore.s */
int	spl0(void);

/* intr.c */
void	intr_init(void);
void	intr_establish(struct isr *, const char *);
void	intr_disestablish(struct isr *);
void	intr_dispatch(int);
void	intr_printlevels(void);
#endif /* _KERNEL */

#endif /* _HP300_INTR_H_ */
