
/*
 * Copyright (c) 2007 Jasper Lievisse Adriaanse <jasper@openbsd.org>
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/signalvar.h>

#include <dev/ic/ds1687reg.h>

#include <machine/bus.h>
#include <machine/autoconf.h>

#include <mips64/archtype.h>
#include <mips64/dev/clockvar.h>

#include <sgi/localbus/crimebus.h>
#include <sgi/localbus/macebus.h>

/*
 * Power button driver for the SGI O2.
 */

struct power_softc {
	struct device		sc_dev;
	bus_space_tag_t		sc_st;
	bus_space_handle_t	sc_sh;
	bus_space_handle_t	sc_isash;
};

int	power_match(struct device *, void *, void *);
void	power_attach(struct device *, struct device *, void *);
int	power_intr(void *aux);

struct cfattach power_ca = {
	sizeof(struct power_softc),
	power_match,
	power_attach
};

struct cfdriver power_cd = {
	NULL, "power", DV_DULL
};

int
power_match(struct device *parent, void *match, void *aux)
{
	return (1);
}

void
power_attach(struct device *parent, struct device *self, void *aux)
{
	struct power_softc *sc = (void *)self;
	struct confargs *ca = aux;
	extern bus_space_handle_t clock_h;
	extern bus_space_handle_t mace_h;

	sc->sc_st = ca->ca_iot;

	/* Map subregion to clock address space. */
	if (bus_space_subregion(sc->sc_st, clock_h, 0, 0x50, &sc->sc_sh)) {
		printf(": failed to map clock address space!\n");
		return;
	}

	/* Map subregion to ISA control registers. */
	if (bus_space_subregion(sc->sc_st, mace_h, 0, 0x80, &sc->sc_isash)) {
		printf(": failed to map ISA control registers!\n");
		return;
	}
 	
	/* Establish interrupt handler. */
	if (macebus_intr_establish(NULL, ca->ca_intr, IST_EDGE, IPL_TTY,
	    power_intr, sc, sc->sc_dev.dv_xname))
		printf("\n");
	else
		printf(": unable to establish interrupt!\n");
}

int
power_intr(void *arg)
{
	struct power_softc *sc = (void *)arg;
	u_int64_t val;
	extern int kbd_reset;

	/* Check to see if this interrupt is for us. */
	val = bus_space_read_8(sc->sc_st, sc->sc_isash, MACE_ISA_INT_STAT);
	if (val & MACE_ISA_INT_RTC) {

		/* 
		 * Prevent further interrupts by clearing the kickstart flag
		 * in the DS1687's extended control register.
		 */
		val = bus_space_read_1(sc->sc_st, sc->sc_sh, DS1687_EXT_CTRL);
		bus_space_write_1(sc->sc_st, sc->sc_sh, DS1687_EXT_CTRL,
		    val & ~DS1687_KICKSTART);

		if (kbd_reset == 1) {
			kbd_reset = 0;
			psignal(initproc, SIGUSR2);
		}

		return (1);
	}

	return (0);
}

