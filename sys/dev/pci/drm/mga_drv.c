/* mga_drv.c -- Matrox G200/G400 driver -*- linux-c -*-
 * Created: Mon Dec 13 01:56:22 1999 by jhartmann@precisioninsight.com
 */
/*-
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Rickard E. (Rik) Faith <faith@valinux.com>
 *    Gareth Hughes <gareth@valinux.com>
 *
 */

#include "drmP.h"
#include "drm.h"
#include "mga_drm.h"
#include "mga_drv.h"
#include "drm_pciids.h"

int	mga_driver_device_is_agp(struct drm_device * );
void	mga_configure(struct drm_device *);

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t mga_pciidlist[] = {
	mga_PCI_IDS
};

/**
 * Determine if the device really is AGP or not.
 *
 * In addition to the usual tests performed by \c drm_device_is_agp, this
 * function detects PCI G450 cards that appear to the system exactly like
 * AGP G450 cards.
 *
 * \param dev   The device to be tested.
 *
 * \returns
 * If the device is a PCI G450, zero is returned.  Otherwise non-zero is
 * returned.
 *
 * \bug
 * This function needs to be filled in!  The implementation in
 * linux-core/mga_drv.c shows what needs to be done.
 */
int
mga_driver_device_is_agp(struct drm_device * dev)
{
#ifdef __FreeBSD__
	device_t bus;

	/* There are PCI versions of the G450.  These cards have the
	 * same PCI ID as the AGP G450, but have an additional PCI-to-PCI
	 * bridge chip.  We detect these cards, which are not currently
	 * supported by this driver, by looking at the device ID of the
	 * bus the "card" is on.  If vendor is 0x3388 (Hint Corp) and the
	 * device is 0x0021 (HB6 Universal PCI-PCI bridge), we reject the
	 * device.
	 */
#if __FreeBSD_version >= 700010
	bus = device_get_parent(device_get_parent(dev->device));
#else
	bus = device_get_parent(dev->device);
#endif
	if (pci_get_device(dev->device) == 0x0525 &&
	    pci_get_vendor(bus) == 0x3388 &&
	    pci_get_device(bus) == 0x0021)
		return DRM_IS_NOT_AGP;
	else
#endif /* XXX Fixme for non freebsd */
		return DRM_MIGHT_BE_AGP;

}

void
mga_configure(struct drm_device *dev)
{
	dev->driver.buf_priv_size	= sizeof(drm_mga_buf_priv_t);
	dev->driver.load		= mga_driver_load;
	dev->driver.unload		= mga_driver_unload;
	dev->driver.lastclose		= mga_driver_lastclose;
	dev->driver.enable_vblank	= mga_enable_vblank;
	dev->driver.disable_vblank	= mga_disable_vblank;
	dev->driver.get_vblank_counter	= mga_get_vblank_counter;
	dev->driver.irq_preinstall	= mga_driver_irq_preinstall;
	dev->driver.irq_postinstall	= mga_driver_irq_postinstall;
	dev->driver.irq_uninstall	= mga_driver_irq_uninstall;
	dev->driver.irq_handler		= mga_driver_irq_handler;
	dev->driver.dma_ioctl		= mga_dma_buffers;
	dev->driver.dma_quiescent	= mga_driver_dma_quiescent;
	dev->driver.device_is_agp	= mga_driver_device_is_agp;

	dev->driver.ioctls		= mga_ioctls;
	dev->driver.max_ioctl		= mga_max_ioctl;

	dev->driver.name		= DRIVER_NAME;
	dev->driver.desc		= DRIVER_DESC;
	dev->driver.date		= DRIVER_DATE;
	dev->driver.major		= DRIVER_MAJOR;
	dev->driver.minor		= DRIVER_MINOR;
	dev->driver.patchlevel		= DRIVER_PATCHLEVEL;

	dev->driver.use_agp		= 1;
	dev->driver.require_agp		= 1;
	dev->driver.use_mtrr		= 1;
	dev->driver.use_dma		= 1;
	dev->driver.use_irq		= 1;
	dev->driver.use_vbl_irq		= 1;
}



#ifdef __FreeBSD__
static int
mga_probe(device_t dev)
{
	return drm_probe(dev, mga_pciidlist);
}

static int
mga_attach(device_t nbdev)
{
	struct drm_device *dev = device_get_softc(nbdev);

	bzero(dev, sizeof(struct drm_device));
	mga_configure(dev);
	return drm_attach(nbdev, mga_pciidlist);
}

static device_method_t mga_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		mga_probe),
	DEVMETHOD(device_attach,	mga_attach),
	DEVMETHOD(device_detach,	drm_detach),

	{ 0, 0 }
};

static driver_t mga_driver = {
	"drm",
	mga_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
#if __FreeBSD_version >= 700010
DRIVER_MODULE(mga, vgapci, mga_driver, drm_devclass, 0, 0);
#else
DRIVER_MODULE(mga, pci, mga_driver, drm_devclass, 0, 0);
#endif
MODULE_DEPEND(mga, drm, 1, 1, 1);

#elif defined(__NetBSD__) || defined(__OpenBSD__)

int	mgadrm_probe(struct device *, void *, void *);
void	mgadrm_attach(struct device *, struct device *, void *);
int
#if defined(__OpenBSD__)
mgadrm_probe(struct device *parent, void *match, void *aux)
#else
mgadrm_probe(struct device *parent, struct cfdata *match, void *aux)
#endif
{
	return drm_probe((struct pci_attach_args *)aux, mga_pciidlist);
}

void
mgadrm_attach(struct device *parent, struct device *self, void *aux)
{
	struct pci_attach_args *pa = aux;
	struct drm_device *dev = (struct drm_device *)self;

	mga_configure(dev);
	return drm_attach(parent, self, pa, mga_pciidlist);
}

#if defined(__OpenBSD__)
struct cfattach mgadrm_ca = {
	sizeof(struct drm_device), mgadrm_probe, mgadrm_attach,
	drm_detach, drm_activate
};

struct cfdriver mgadrm_cd = {
	0, "mgadrm", DV_DULL
};
#else
#ifdef _LKM
CFDRIVER_DECL(mgadrm, DV_TTY, NULL);
#else
CFATTACH_DECL(mgadrm, sizeof(struct drm_device), mgadrm_probe, mgadrm_attach,
    drm_detach, drm_activate);
#endif
#endif

#endif
