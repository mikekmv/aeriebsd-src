/*
 * Copyright (c) 2009 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * driver for the Micrel KSZ8041NL 10/100 PHY
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/if_media.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include <dev/mii/miidevs.h>

/* register definitions */
#define	KSZ_RXERR	0x15	/* recv error count for sym-err */
#define	KSZ_ICR		0x1b	/* int control/status */
#define	KSZ_PHYCTL0	0x1e	/* PHY control reg0 */
#define	KSZ_CTL0_LEDSPLA	0x0000	/* rw led1 = speed; led0 = link/act */
#define	KSZ_CTL0_LEDAL		0x4000	/* rw led1 = act; led0 = link */
#define	KSZ_CTL0_POLARITY	0x2000	/* ro polarity is reversed */
#define	KSZ_CTL0_MDIX		0x0800	/* ro mdi/mdi-x */
#define	KSZ_CTL0_RMTLOOP	0x0080	/* rw remote loopback */
#define	KSZ_PHYCTL1	0x1f	/* PHY control reg1 */
#define	KSZ_CTL1_HPMDIX		0x8000	/* rw hp mdi-x mode */
#define	KSZ_CTL1_MDIX		0x4000	/* rw mdi/mdi-x (when non-auto) */
#define	KSZ_CTL1_AUTOMDIX	0x2000	/* rw auto mdi/mdi-x */
#define	KSZ_CTL1_ENEDETECT	0x1000	/* ro signal detected on rx+/- */
#define	KSZ_CTL1_FORCELINK	0x0800	/* rw force link pass */
#define	KSZ_CTL1_PWRSAVE	0x0400	/* rw disable rxclk on no cable */
#define	KSZ_CTL1_INTLVL		0x0200	/* rw int level high */
#define	KSZ_CTL1_ENAJAB		0x0100	/* rw enable jabber counter */
#define	KSZ_CTL1_ANEGCMPL	0x0080	/* rw auto-neg is complete */
#define	KSZ_CTL1_ENAPAUSE	0x0040	/* ro flow-control capable */
#define	KSZ_CTL1_ISOLATE	0x0020	/* ro PHY is isolated */
#define	KSZ_CTL1_OPMODE		0x001c	/* ro current op mode */
#define	KSZ_CTL1_ENASQE		0x0002	/* rw enable SQE test */
#define	KSZ_CTL1_DISSCRA	0x0001	/* rw disable data scrambler  */

struct cfdriver kszphy_cd = {
	NULL, "kszphy", DV_DULL
};

int	kszphymatch(struct device *, void *, void *);
void    kszphyattach(struct device *, struct device *, void *);

struct cfattach kszphy_ca = {
	sizeof(struct mii_softc), kszphymatch, kszphyattach, mii_phy_detach,
	    mii_phy_activate
};

static const struct mii_phydesc kszphys[] = {
	{ MII_OUI_MICREL,
	  MII_MODEL_MICREL_KSZ8041NL, MII_STR_MICREL_KSZ8041NL },

	{ 0 }
};

int	kszphy_service(struct mii_softc *, struct mii_data *, int);
void	kszphy_reset(struct mii_softc *);

const struct mii_phy_funcs kszphy_funcs = {
	kszphy_service, ukphy_status, kszphy_reset,
};

int
kszphymatch(struct device *parent, void *match, void *aux)
{
        struct mii_attach_args *ma = aux;

	if (mii_phy_match(ma, kszphys) != NULL)
		return (10);

	return (0);
}

void
kszphyattach(struct device *parent, struct device *self, void *aux)
{
	struct mii_softc *sc = (struct mii_softc *)self;
	struct mii_attach_args *ma = aux;
	struct mii_data *mii = ma->mii_data;
	const struct mii_phydesc *mpd;

	mpd = mii_phy_match(ma, kszphys);
	printf(": %s, rev. %d\n", mpd->mpd_name, MII_REV(ma->mii_id2));

	sc->mii_inst = mii->mii_instance;
	sc->mii_phy = ma->mii_phyno;
	sc->mii_funcs = &kszphy_funcs;
	sc->mii_pdata = mii;
	sc->mii_flags = ma->mii_flags;

	PHY_RESET(sc);

	sc->mii_capabilities =
	    PHY_READ(sc, MII_BMSR) & ma->mii_capmask;
	if (sc->mii_capabilities & BMSR_MEDIAMASK)
		mii_phy_add_media(sc);
}

int
kszphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
{
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;

	if ((sc->mii_dev.dv_flags & DVF_ACTIVE) == 0)
		return (ENXIO);

	switch (cmd) {
	case MII_POLLSTAT:
		/* If we're not polling our PHY instance, just return. */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst)
			return (0);
		break;

	case MII_MEDIACHG:
		if (IFM_INST(ife->ifm_media) != sc->mii_inst) {
			PHY_WRITE(sc, MII_BMCR,
			    PHY_READ(sc, MII_BMCR) | BMCR_ISO);
			return (0);
		}

		if ((mii->mii_ifp->if_flags & IFF_UP) == 0)
			break;

		if (IFM_SUBTYPE(ife->ifm_media) == IFM_NONE)
			PHY_WRITE(sc, MII_BMCR, BMCR_ISO|BMCR_PDOWN);
		break;

	case MII_TICK:
		/* If we're not currently selected, just return. */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst)
			return (0);

		if (mii_phy_tick(sc) == EJUSTRETURN)
			return (0);
		break;

	case MII_DOWN:
		mii_phy_down(sc);
		return (0);
	}

	mii_phy_status(sc);
	mii_phy_update(sc, cmd);
	return (0);
}

void
kszphy_reset(struct mii_softc *sc)
{
	mii_phy_reset(sc);
	PHY_WRITE(sc, KSZ_PHYCTL1,
	    PHY_READ(sc, KSZ_PHYCTL1) | KSZ_CTL1_PWRSAVE);
}
