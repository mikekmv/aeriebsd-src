
/*
 * Copyright (c) 2001-2004 Opsycon AB  (www.opsycon.se / www.opsycon.com)
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _MACHINE_INTR_H_
#define _MACHINE_INTR_H_

/*
 *  The interrupt mask cpl is a mask which can be used with the
 *  CPU interrupt mask register or an external HW mask register.
 *  If interrupts are masked by the CPU interrupt mask all external
 *  masks should be enabled and any routing set up so that the
 *  interrupt source is routed to the CPU interrupt corresponding
 *  to the interrupts "priority level". In this case the generic
 *  interrupt handler can be used.
 *
 *  The IMASK_EXTERNAL define is used to select whether the CPU
 *  interrupt mask should be controlled by the cpl mask value
 *  or not. If the mask is external, the CPU mask is never changed
 *  from the value it gets when interrupt dispatchers are registered.
 *  When an external masking register is used dedicated interrupt
 *  handlers must be written as well as ipending handlers.
 */
#define	IMASK_EXTERNAL		/* XXX move this to config */

/* This define controls whether splraise is inlined or not */
/* #define INLINE_SPLRAISE */


/* Interrupt priority `levels'; not mutually exclusive. */
#define	IPL_NONE	0	/* nothing */
#define	IPL_BIO		1	/* block I/O */
#define IPL_AUDIO	IPL_BIO
#define	IPL_NET		2	/* network */
#define	IPL_TTY		3	/* terminal */
#define	IPL_VM		4	/* memory allocation */
#define	IPL_CLOCK	5	/* clock */
#define	IPL_HIGH	6	/* everything */
#define	NIPLS		7	/* Number of levels */

/* Interrupt sharing types. */
#define	IST_NONE	0	/* none */
#define	IST_PULSE	1	/* pulsed */
#define	IST_EDGE	2	/* edge-triggered */
#define	IST_LEVEL	3	/* level-triggered */

/* Soft interrupt masks. */
#define	SINT_CLOCK	31
#define	SINT_CLOCKMASK	(1 << SINT_CLOCK)
#define	SINT_NET	30
#define	SINT_NETMASK	((1 << SINT_NET) | SINT_CLOCKMASK)
#define	SINT_TTY	29
#define	SINT_TTYMASK	((1 << SINT_TTY) | SINT_CLOCKMASK)
#define	SINT_ALLMASK	(SINT_CLOCKMASK | SINT_NETMASK | SINT_TTYMASK)
#define	SPL_CLOCK	28
#define	SPL_CLOCKMASK	(1 << SPL_CLOCK)

#ifndef _LOCORE

#define splbio()		splraise(imask[IPL_BIO])
#define splnet()		splraise(imask[IPL_NET])
#define spltty()		splraise(imask[IPL_TTY])
#define splaudio()		splraise(imask[IPL_AUDIO])
#define splclock()		splraise(imask[IPL_CLOCK])
#define splvm()			splraise(imask[IPL_VM])
#define splsoftclock()		splraise(SINT_CLOCKMASK)
#define splsoftnet()		splraise(SINT_NETMASK|SINT_CLOCKMASK)
#define splsofttty()		splraise(SINT_TTYMASK)
#define splstatclock()		splhigh()
#define splsched()		splhigh()
#define splhigh()		splraise(-1)
#define spl0()			spllower(0)

#include <machine/atomic.h>

#define setsoftclock()  	atomic_setbits_int(&ipending, SINT_CLOCKMASK)
#define setsoftnet()    	atomic_setbits_int(&ipending, SINT_NETMASK)
#define setsofttty()    	atomic_setbits_int(&ipending, SINT_TTYMASK)

void	splinit(void);

#define	splassert(X)

/*
 *  Schedule prioritys for base interrupts (CPU)
 */
#define	INTPRI_CLOCK	1
#define	INTPRI_MACEIO	2	/* O2 I/O interrupt */
#define	INTPRI_XBOWMUX	2	/* Origin 200/2000 I/O interrupt */
#define	INTPRI_MACEAUX	3

/*
 *   Define a type for interrupt masks. We may need 64 bits here.
 */
typedef u_int32_t intrmask_t;		/* Type of var holding interrupt mask */

#define	INTMASKSIZE	(sizeof(intrmask_t) * 8)

extern volatile intrmask_t cpl;
extern volatile intrmask_t ipending;
extern volatile intrmask_t astpending;

extern intrmask_t imask[NIPLS];

/*
 *  A note on clock interrupts. Clock interrupts are always
 *  allowed to happen but will not be serviced if masked.
 *  The reason for this is that clocks usually sit on INT5
 *  and cannot be easily masked if external HW masking is used.
 */

/* Inlines */
static __inline void register_pending_int_handler(void (*)(int));
static __inline void splx(int newcpl);
static __inline int spllower(int newcpl);

typedef void  (int_f) (int);
extern int_f *pending_hand;

static __inline void
register_pending_int_handler(void(*pending)(int))
{
	pending_hand = pending;
}

/*
 */
#ifdef INLINE_SPLRAISE
static __inline int splraise(int newcpl);
static __inline int
splraise(int newcpl)
{
	int oldcpl;

	__asm__ (" .set noreorder\n");
	oldcpl = cpl;
	cpl = oldcpl | newcpl;
	__asm__ (" sync\n .set reorder\n");
	return (oldcpl);
}
#else
int splraise(int newcpl);
#endif

static __inline void
splx(int newcpl)
{
	if (ipending & ~newcpl)
		(*pending_hand)(newcpl);
	else
		cpl = newcpl;
}

static __inline int
spllower(int newcpl)
{
	int oldcpl;

	oldcpl = cpl;
	if (ipending & ~newcpl)
		(*pending_hand)(newcpl);
	else
		cpl = newcpl;
	return (oldcpl);
}

/*
 * Interrupt control struct used by interrupt dispatchers
 * to hold interrupt handler info.
 */

#include <sys/evcount.h>

struct intrhand {
	struct	intrhand *ih_next;
	int	(*ih_fun)(void *);
	void	*ih_arg;
	int	ih_level;
	int	ih_irq;
	char	*ih_what;
	void	*frame;
	struct evcount  ih_count;
};

extern struct intrhand *intrhand[INTMASKSIZE];

/*
 * Low level interrupt dispatcher registration data.
 */
#define NLOWINT	16		/* Number of low level registrations possible */

struct trap_frame;

extern intrmask_t idle_mask;
extern int last_low_int;

void set_intr(int, intrmask_t, intrmask_t(*)(intrmask_t, struct trap_frame *));

#ifdef IMASK_EXTERNAL
void hw_setintrmask(intrmask_t);
extern void *hwmask_addr;
#endif

/*
 *  Generic interrupt handling code that can be used for simple
 *  interrupt hardware models. Functions can also be used by
 *  more complex code especially the mask calculation code.
 */

void *generic_intr_establish(void *, u_long, int, int,
	    int (*)(void *), void *, char *);
void generic_intr_disestablish(void *, void *);
void generic_intr_makemasks(void);
void generic_do_pending_int(int);
intrmask_t generic_iointr(intrmask_t, struct trap_frame *);

#endif /* _LOCORE */

#endif /* _MACHINE_INTR_H_ */
