
/*
 * Copyright (c) 2000-2004 Opsycon AB  (www.opsycon.se)
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

/*
 * This is a combined macebus/crimebus driver. It handles configuration of all
 * devices on the processor bus.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/tty.h>
#include <sys/extent.h>
#include <net/netisr.h>
#include <uvm/uvm_extern.h>

#include <dev/pci/pcidevs.h>
#include <dev/pci/pcivar.h>

#include <dev/ic/comvar.h>

#include <mips64/archtype.h>

#include <machine/autoconf.h>
#include <machine/intr.h>
#include <machine/atomic.h>

#include <sgi/localbus/crimebus.h>
#include <sgi/localbus/macebus.h>

int	macebusmatch(struct device *, void *, void *);
void	macebusattach(struct device *, struct device *, void *);
int	macebusprint(void *, const char *);
int	macebussearch(struct device *, void *, void *);

void	macebus_intr_makemasks(void);
void	macebus_do_pending_int(int);
intrmask_t macebus_iointr(intrmask_t, struct trap_frame *);
intrmask_t macebus_aux(intrmask_t, struct trap_frame *);

u_int8_t mace_read_1(bus_space_tag_t, bus_space_handle_t, bus_size_t);
u_int16_t mace_read_2(bus_space_tag_t, bus_space_handle_t, bus_size_t);
u_int32_t mace_read_4(bus_space_tag_t, bus_space_handle_t, bus_size_t);
u_int64_t mace_read_8(bus_space_tag_t, bus_space_handle_t, bus_size_t);

void mace_write_1(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_int8_t);
void mace_write_2(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_int16_t);
void mace_write_4(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_int32_t);
void mace_write_8(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_int64_t);

void mace_read_raw_2(bus_space_tag_t, bus_space_handle_t,
	    bus_addr_t, u_int8_t *, bus_size_t);
void mace_write_raw_2(bus_space_tag_t, bus_space_handle_t,
	    bus_addr_t, const u_int8_t *, bus_size_t);
void mace_read_raw_4(bus_space_tag_t, bus_space_handle_t,
	    bus_addr_t, u_int8_t *, bus_size_t);
void mace_write_raw_4(bus_space_tag_t, bus_space_handle_t,
	    bus_addr_t, const u_int8_t *, bus_size_t);
void mace_read_raw_8(bus_space_tag_t, bus_space_handle_t,
	    bus_addr_t, u_int8_t *, bus_size_t);
void mace_write_raw_8(bus_space_tag_t, bus_space_handle_t,
	    bus_addr_t, const u_int8_t *, bus_size_t);

int mace_space_map(bus_space_tag_t, bus_addr_t, bus_size_t, int, bus_space_handle_t *);
void mace_space_unmap(bus_space_tag_t, bus_space_handle_t, bus_size_t);
int mace_space_region(bus_space_tag_t, bus_space_handle_t, bus_size_t, bus_size_t, bus_space_handle_t *);

void *mace_space_vaddr(bus_space_tag_t, bus_space_handle_t);

bus_addr_t macebus_pa_to_device(paddr_t);
paddr_t	macebus_device_to_pa(bus_addr_t);

int maceticks;		/* Time tracker for special events. */

struct cfattach macebus_ca = {
	sizeof(struct device), macebusmatch, macebusattach
};

struct cfdriver macebus_cd = {
	NULL, "macebus", DV_DULL
};

bus_space_t macebus_tag = {
	NULL,
	(bus_addr_t)MACEBUS_BASE,
	NULL,
	0,
	mace_read_1, mace_write_1,
	mace_read_2, mace_write_2,
	mace_read_4, mace_write_4,
	mace_read_8, mace_write_8,
	mace_read_raw_2, mace_write_raw_2,
	mace_read_raw_4, mace_write_raw_4,
	mace_read_raw_8, mace_write_raw_8,
	mace_space_map, mace_space_unmap, mace_space_region,
	mace_space_vaddr
};

bus_space_t crimebus_tag = {
	NULL,
	(bus_addr_t)CRIMEBUS_BASE,
	NULL,
	0,
	mace_read_1, mace_write_1,
	mace_read_2, mace_write_2,
	mace_read_4, mace_write_4,
	mace_read_8, mace_write_8,
	mace_read_raw_2, mace_write_raw_2,
	mace_read_raw_4, mace_write_raw_4,
	mace_read_raw_8, mace_write_raw_8,
	mace_space_map, mace_space_unmap, mace_space_region,
	mace_space_vaddr
};

bus_space_handle_t crime_h;
bus_space_handle_t mace_h;

struct machine_bus_dma_tag mace_bus_dma_tag = {
	NULL,			/* _cookie */
	_dmamap_create,
	_dmamap_destroy,
	_dmamap_load,
	_dmamap_load_mbuf,
	_dmamap_load_uio,
	_dmamap_load_raw,
	_dmamap_unload,
	_dmamap_sync,
	_dmamem_alloc,
	_dmamem_free,
	_dmamem_map,
	_dmamem_unmap,
	_dmamem_mmap,
	macebus_pa_to_device,
	macebus_device_to_pa,
	CRIME_MEMORY_MASK
};

/*
 * Match bus only to targets which have this bus.
 */
int
macebusmatch(struct device *parent, void *match, void *aux)
{
	if (sys_config.system_type == SGI_O2)
		return (1);
	return (0);
}

int
macebusprint(void *aux, const char *macebus)
{
	struct confargs *ca = aux;

	if (macebus != NULL)
		printf("%s at %s", ca->ca_name, macebus);

	if (ca->ca_baseaddr != 0)
		printf(" base 0x%08x", ca->ca_baseaddr);
	if (ca->ca_intr != 0)
		printf(" irq %d", ca->ca_intr);

	return (UNCONF);
}

int
macebussearch(struct device *parent, void *child, void *args)
{
	struct cfdata *cf = child;
	struct confargs ca;

	ca.ca_name = cf->cf_driver->cd_name;
	ca.ca_iot = &macebus_tag;
	ca.ca_memt = &macebus_tag;
	ca.ca_dmat = &mace_bus_dma_tag;
	if (cf->cf_loc[0] == -1)
		ca.ca_baseaddr = 0;
	else
		ca.ca_baseaddr = cf->cf_loc[0];
	if (cf->cf_loc[1] == -1)
		ca.ca_intr = 0;
	else
		ca.ca_intr = cf->cf_loc[1];

	if ((*cf->cf_attach->ca_match)(parent, cf, &ca) == 0)
		return (0);

	config_attach(parent, cf, &ca, macebusprint);
	return (1);
}

void
macebusattach(struct device *parent, struct device *self, void *aux)
{
	u_int32_t creg;
	u_int64_t mask;

	/*
	 * Create an extent for the localbus control registers.
	 */
	macebus_tag.bus_extent = extent_create("mace_space",
	    macebus_tag.bus_base, macebus_tag.bus_base + 0x00400000,
	    M_DEVBUF, NULL, 0, EX_NOCOALESCE | EX_NOWAIT);

	crimebus_tag.bus_extent = extent_create("crime_space",
	    crimebus_tag.bus_base, crimebus_tag.bus_base + 0x03000000,
	    M_DEVBUF, NULL, 0, EX_NOCOALESCE | EX_NOWAIT);

	/*
	 * Map and setup CRIME control registers.
	 */
	if (bus_space_map(&crimebus_tag, 0x00000000, 0x400, 0, &crime_h)) {
		printf(": cannot map CRIME control registers\n");
		return;
	}
	hwmask_addr = (void *)(PHYS_TO_UNCACHED(CRIMEBUS_BASE) +
	    CRIME_INT_MASK);

	creg = bus_space_read_8(&crimebus_tag, crime_h, CRIME_REVISION);
	printf(": crime rev %d.%d\n", (creg & 0xf0) >> 4, creg & 0xf);

	bus_space_write_8(&crimebus_tag, crime_h, CRIME_CPU_ERROR_STAT, 0);
	bus_space_write_8(&crimebus_tag, crime_h, CRIME_MEM_ERROR_STAT, 0);

	mask = 0;
	bus_space_write_8(&crimebus_tag, crime_h, CRIME_INT_MASK, mask);
	bus_space_write_8(&crimebus_tag, crime_h, CRIME_INT_SOFT, 0);
	bus_space_write_8(&crimebus_tag, crime_h, CRIME_INT_HARD, 0);
	bus_space_write_8(&crimebus_tag, crime_h, CRIME_INT_STAT, 0);


	/*
	 * Map and setup MACE ISA control registers.
	 */
	if (bus_space_map(&macebus_tag, MACE_ISA_OFFS, 0x400, 0, &mace_h)) {
		printf("UH-OH! Can't map MACE ISA control registers!\n");
		return;
	}

	/* Turn on all interrupts except for MACE compare/timer. */
	bus_space_write_8(&macebus_tag, mace_h, MACE_ISA_INT_MASK, 
			  0xffffffff & ~MACE_ISA_INT_TIMER);
	bus_space_write_8(&macebus_tag, mace_h, MACE_ISA_INT_STAT, 0);

	/*
	 * On O2 systems all interrupts are handled by the macebus interrupt
	 * handler. Register all except clock.
	 */
	set_intr(INTPRI_MACEIO, CR_INT_0, macebus_iointr);
	register_pending_int_handler(macebus_do_pending_int);

	/* DEBUG: Set up a handler called when clock interrupts go off. */
	set_intr(INTPRI_MACEAUX, CR_INT_5, macebus_aux);

	config_search(macebussearch, self, aux);
}

/*
 * Bus access primitives. These are really ugly...
 */

u_int8_t
mace_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return *(volatile u_int8_t *)(h + (o << 8) + 7);
}

u_int16_t
mace_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	panic("mace_read_2");
}

u_int32_t
mace_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return *(volatile u_int32_t *)(h + o);
}

u_int64_t
mace_read_8(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return *(volatile u_int64_t *)(h + o);
}

void
mace_write_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o, u_int8_t v)
{
	*(volatile u_int8_t *)(h + (o << 8) + 7) = v;
}

void
mace_write_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o, u_int16_t v)
{
	panic("mace_write_2");
}

void
mace_write_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o, u_int32_t v)
{
	*(volatile u_int32_t *)(h + o) = v;
}

void
mace_write_8(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o, u_int64_t v)
{
	*(volatile u_int64_t *)(h + o) = v;
}

void
mace_read_raw_2(bus_space_tag_t t, bus_space_handle_t h, bus_addr_t o,
    u_int8_t *buf, bus_size_t len)
{
	panic(__func__);
}

void
mace_write_raw_2(bus_space_tag_t t, bus_space_handle_t h, bus_addr_t o,
    const u_int8_t *buf, bus_size_t len)
{
	panic(__func__);
}

void
mace_read_raw_4(bus_space_tag_t t, bus_space_handle_t h, bus_addr_t o,
    u_int8_t *buf, bus_size_t len)
{
	volatile u_int32_t *addr = (volatile u_int32_t *)(h + o);
	len >>= 2;
	while (len-- != 0) {
		*(u_int32_t *)buf = *addr;
		buf += 4;
	}
}

void
mace_write_raw_4(bus_space_tag_t t, bus_space_handle_t h, bus_addr_t o,
    const u_int8_t *buf, bus_size_t len)
{
	volatile u_int32_t *addr = (volatile u_int32_t *)(h + o);
	len >>= 2;
	while (len-- != 0) {
		*addr = *(u_int32_t *)buf;
		buf += 4;
	}
}

void
mace_read_raw_8(bus_space_tag_t t, bus_space_handle_t h, bus_addr_t o,
    u_int8_t *buf, bus_size_t len)
{
	volatile u_int64_t *addr = (volatile u_int64_t *)(h + o);
	len >>= 3;
	while (len-- != 0) {
		*(u_int64_t *)buf = *addr;
		buf += 8;
	}
}

void
mace_write_raw_8(bus_space_tag_t t, bus_space_handle_t h, bus_addr_t o,
    const u_int8_t *buf, bus_size_t len)
{
	volatile u_int64_t *addr = (volatile u_int64_t *)(h + o);
	len >>= 3;
	while (len-- != 0) {
		*addr = *(u_int64_t *)buf;
		buf += 8;
	}
}

extern int extent_malloc_flags;

int
mace_space_map(bus_space_tag_t t, bus_addr_t offs, bus_size_t size,
    int cacheable, bus_space_handle_t *bshp)
{
	bus_addr_t bpa;
	int error;

	bpa = t->bus_base + offs;

	/* Handle special mapping separately. */
	if ((bpa >= (MACEBUS_BASE + MACE_IO_OFFS) &&
	    (bpa + size) < (MACEBUS_BASE + MACE_IO_OFFS + MACE_IO_SIZE))
	    || (bpa >= (MACEBUS_BASE + MACE_ISAX_OFFS) &&
	    (bpa + size) < (MACEBUS_BASE + MACE_ISAX_OFFS + MACE_ISAX_SIZE))) {
		*bshp = PHYS_TO_UNCACHED(bpa);
		return 0;
	}

	if ((error = extent_alloc_region(t->bus_extent, bpa, size,
	    EX_NOWAIT | extent_malloc_flags))) {
		return error;
	}

	if ((error = bus_mem_add_mapping(bpa, size, cacheable, bshp))) {
		if (extent_free(t->bus_extent, bpa, size,
		    EX_NOWAIT | extent_malloc_flags)) {
			printf("bus_space_map: pa %p, size %p\n", bpa, size);
			printf("bus_space_map: can't free region\n");
		}
	}

	return (error);
}

void
mace_space_unmap(bus_space_tag_t t, bus_space_handle_t bsh, bus_size_t size)
{
	bus_addr_t sva, paddr;
	bus_size_t off, len;

	/* Should this verify that the proper size is freed? */
	sva = trunc_page(bsh);
	off = bsh - sva;
	len = size+off;

	if (IS_XKPHYS(bsh)) {
		paddr = XKPHYS_TO_PHYS(bsh);
		if ((paddr >= (MACEBUS_BASE + MACE_IO_OFFS) &&
		    (paddr + size) <=
		    (MACEBUS_BASE + MACE_IO_OFFS + MACE_IO_SIZE))
		    || (paddr >= (MACEBUS_BASE + MACE_ISAX_OFFS) &&
		    (paddr + size) <=
		    (MACEBUS_BASE + MACE_ISAX_OFFS + MACE_ISAX_SIZE)))
			return;
	}

	if (pmap_extract(pmap_kernel(), bsh, (void *)&paddr) == 0) {
		printf("bus_space_unmap: no pa for va %p\n", bsh);
		return;
	}

	uvm_km_free(kernel_map, sva, len);

	if (extent_free(t->bus_extent, paddr, size,
	    EX_NOWAIT | extent_malloc_flags)) {
		printf("bus_space_map: pa %p, size %p\n", paddr, size);
		printf("bus_space_map: can't free region\n");
	}
}

int
mace_space_region(bus_space_tag_t t, bus_space_handle_t bsh,
    bus_size_t offset, bus_size_t size, bus_space_handle_t *nbshp)
{
	*nbshp = bsh + offset;
	return (0);
}

void *
mace_space_vaddr(bus_space_tag_t t, bus_space_handle_t h)
{
	return (void *)h;
}

/*
 * Macebus bus_dma helpers.
 * Mace accesses memory contiguously at 0x40000000 onwards.
 */

bus_addr_t
macebus_pa_to_device(paddr_t pa)
{
	return (pa | CRIME_MEMORY_OFFSET);
}

paddr_t
macebus_device_to_pa(bus_addr_t addr)
{
	paddr_t pa = (paddr_t)addr & CRIME_MEMORY_MASK;

	if (pa >= 256 * 1024 * 1024)
		pa |= CRIME_MEMORY_OFFSET;

	return (pa);
}

/*
 * Macebus interrupt handler driver.
 */

intrmask_t mace_intem = 0x0;
static intrmask_t intrtype[INTMASKSIZE];
static intrmask_t intrmask[INTMASKSIZE];
static intrmask_t intrlevel[INTMASKSIZE];

static int fakeintr(void *);
static int fakeintr(void *a) {return 0;}

/*
 * Establish an interrupt handler called from the dispatcher.
 * The interrupt function established should return zero if there was nothing
 * to serve (no int) and non-zero when an interrupt was serviced.
 * Interrupts are numbered from 1 and up where 1 maps to HW int 0.
 */
void *
macebus_intr_establish(void *icp, u_long irq, int type, int level,
    int (*ih_fun)(void *), void *ih_arg, char *ih_what)
{
	struct intrhand **p, *q, *ih;
	static struct intrhand fakehand = {NULL, fakeintr};
	int edge;
	extern int cold;
	static int initialized = 0;

	if (!initialized) {
		/*INIT CODE HERE*/
		initialized = 1;
	}

	if (irq > 62 || irq < 1) {
		panic("intr_establish: illegal irq %d", irq);
	}
	irq -= 1;	/* Adjust for 1 being first (0 is no int) */

	/* No point in sleeping unless someone can free memory. */
	ih = malloc(sizeof *ih, M_DEVBUF, cold ? M_NOWAIT : M_WAITOK);
	if (ih == NULL)
		panic("intr_establish: can't malloc handler info");

	if (type == IST_NONE || type == IST_PULSE)
		panic("intr_establish: bogus type");

	switch (intrtype[irq]) {
	case IST_EDGE:
	case IST_LEVEL:
		if (type == intrtype[irq])
			break;
	}

	switch (type) {
	case IST_EDGE:
		edge |= 1 << irq;
		break;
	case IST_LEVEL:
		edge &= ~(1 << irq);
		break;
	}

	/*
	 * Figure out where to put the handler.
	 * This is O(N^2), but we want to preserve the order, and N is
	 * generally small.
	 */
	for (p = &intrhand[irq]; (q = *p) != NULL; p = &q->ih_next)
		;

	/*
	 * Actually install a fake handler momentarily, since we might be doing
	 * this with interrupts enabled and don't want the real routine called
	 * until masking is set up.
	 */
	fakehand.ih_level = level;
	*p = &fakehand;

	macebus_intr_makemasks();

	/*
	 * Poke the real handler in now.
	 */
	ih->ih_fun = ih_fun;
	ih->ih_arg = ih_arg;
	ih->ih_next = NULL;
	ih->ih_level = level;
	ih->ih_irq = irq;
	ih->ih_what = ih_what;
	evcount_attach(&ih->ih_count, ih_what, (void *)&ih->ih_irq,
	    &evcount_intr);
	*p = ih;

	return (ih);
}

void
macebus_intr_disestablish(void *p1, void *p2)
{
}

/*
 * Regenerate interrupt masks to reflect reality.
 */
void
macebus_intr_makemasks(void)
{
	int irq, level;
	struct intrhand *q;

	/* First, figure out which levels each IRQ uses. */
	for (irq = 0; irq < INTMASKSIZE; irq++) {
		int levels = 0;
		for (q = intrhand[irq]; q; q = q->ih_next)
			levels |= 1 << q->ih_level;
		intrlevel[irq] = levels;
	}

	/* Then figure out which IRQs use each level. */
	for (level = IPL_NONE; level < NIPLS; level++) {
		int irqs = 0;
		for (irq = 0; irq < INTMASKSIZE; irq++)
			if (intrlevel[irq] & (1 << level))
				irqs |= 1 << irq;
		imask[level] = irqs | SINT_ALLMASK;
	}

	/*
	 * There are tty, network and disk drivers that use free() at interrupt
	 * time, so imp > (tty | net | bio).
	 *
	 * Enforce a hierarchy that gives slow devices a better chance at not
	 * dropping data.
	 */
	imask[IPL_NET] |= imask[IPL_BIO];
	imask[IPL_TTY] |= imask[IPL_NET];
	imask[IPL_VM] |= imask[IPL_TTY];
	imask[IPL_CLOCK] |= imask[IPL_VM] | SPL_CLOCKMASK;

	/*
	 * These are pseudo-levels.
	 */
	imask[IPL_NONE] = 0;
	imask[IPL_HIGH] = -1;

	/* And eventually calculate the complete masks. */
	for (irq = 0; irq < INTMASKSIZE; irq++) {
		int irqs = 1 << irq;
		for (q = intrhand[irq]; q; q = q->ih_next)
			irqs |= imask[q->ih_level];
		intrmask[irq] = irqs | SINT_ALLMASK;
	}

	/* Lastly, determine which IRQs are actually in use. */
	irq = 0;
	for (level = 0; level < INTMASKSIZE; level++) {
		if (intrhand[level]) {
			irq |= 1 << level;
		}
	}
	mace_intem = irq & 0x0000ffff;
	hw_setintrmask(0);
}

void
macebus_do_pending_int(int newcpl)
{
#if 0
	struct intrhand *ih;
	int vector;
	intrmask_t hwpend;
	struct trap_frame cf;
	static volatile int processing;

	/* Don't recurse... but change the mask. */
	if (processing) {
		__asm__ (" .set noreorder\n");
		cpl = newcpl;
		__asm__ (" sync\n .set reorder\n");
		return;
	}
	processing = 1;


	/* XXX Fake a trapframe for clock pendings... */
	cf.pc = (int)&macebus_do_pending_int;
	cf.sr = 0;
	cf.cpl = cpl;

	/* Hard mask current cpl so we don't get any new pendings. */
	hw_setintrmask(cpl);

	/* Find out what interrupts we should process. */
	hwpend = ipending & ~newcpl;
	hwpend &= ~SINT_ALLMASK;
	atomic_clearbits_int(&ipending, hwpend);

	/* Enable all non-pending non-masked hardware interrupts. */
	__asm__ (" .set noreorder\n");
	cpl = (cpl & SINT_ALLMASK) | (newcpl & ~SINT_ALLMASK) | hwpend;
	__asm__ (" sync\n .set reorder\n");
	hw_setintrmask(cpl);

	while (hwpend) {
		vector = ffs(hwpend) - 1;
		hwpend &= ~(1L << vector);
		ih = intrhand[vector];
		while (ih) {
			ih->frame = &cf;
			if ((*ih->ih_fun)(ih->ih_arg)) {
				ih->ih_count.ec_count++;
			}
			ih = ih->ih_next;
		}
	}

	/* Enable all processed pending hardware interrupts. */
	__asm__ (" .set noreorder\n");
	cpl &= ~hwpend;
	__asm__ (" sync\n .set reorder\n");
	hw_setintrmask(cpl);

	if ((ipending & SINT_CLOCKMASK) & ~newcpl) {
		atomic_clearbits_int(&ipending, SINT_CLOCKMASK);
		softclock();
	}
	if ((ipending & SINT_NETMASK) & ~newcpl) {
		extern int netisr;
		int isr;

		atomic_clearbits_int(&ipending, SINT_NETMASK);
		while ((isr = netisr) != 0) {
			atomic_clearbits_int(&netisr, isr);
#define	DONETISR(b,f)	if (isr & (1 << (b)))	f();
#include <net/netisr_dispatch.h>
		}
	}

#ifdef notyet
	if ((ipending & SINT_TTYMASK) & ~newcpl) {
		atomic_clearbits_int(&ipending, SINT_TTYMASK);
		compoll(NULL);
	}
#endif

	/* Update masks to new cpl. Order highly important! */
	__asm__ (" .set noreorder\n");
	cpl = newcpl;
	__asm__ (" sync\n .set reorder\n");
	hw_setintrmask(newcpl);

	processing = 0;
#else
	/* Update masks to new cpl. Order highly important! */
	__asm__ (" .set noreorder\n");
	cpl = newcpl;
	__asm__ (" sync\n .set reorder\n");
	hw_setintrmask(newcpl);
	/* If we still have softints pending trigger processing. */
	if (ipending & SINT_ALLMASK & ~newcpl)
		setsoftintr0();
#endif
}

/*
 * Process interrupts. The parameter pending has non-masked interrupts.
 */
intrmask_t
macebus_iointr(intrmask_t hwpend, struct trap_frame *cf)
{
	struct intrhand *ih;
	intrmask_t caught, vm;
	int v;
	intrmask_t pending;
	u_int64_t intstat, isastat, mask;
#ifdef DIAGNOSTIC
	static int spurious = 0;
#endif

	intstat = bus_space_read_8(&crimebus_tag, crime_h, CRIME_INT_STAT);
	intstat &= 0xffff;

	isastat = bus_space_read_8(&macebus_tag, mace_h, MACE_ISA_INT_STAT);
	caught = 0;

	/* Mask off masked interrupts and save them as pending. */
	if (intstat & cf->cpl) {
		atomic_setbits_int(&ipending, intstat & cf->cpl);
		mask = bus_space_read_8(&crimebus_tag, crime_h, CRIME_INT_MASK);
		mask &= ~ipending;
		bus_space_write_8(&crimebus_tag, crime_h, CRIME_INT_MASK, mask);
		caught++;
	}

	/* Scan all unmasked. Scan the first 16 for now. */
	pending = intstat & ~cf->cpl;
	atomic_clearbits_int(&ipending, pending);

	for (v = 0, vm = 1; pending != 0 && v < 16 ; v++, vm <<= 1) {
		if (pending & vm) {
			ih = intrhand[v];

			while (ih) {
				ih->frame = cf;
				if ((*ih->ih_fun)(ih->ih_arg)) {
					caught |= vm;
					ih->ih_count.ec_count++;
				}
				ih = ih->ih_next;
			}
		}
	}

	if (caught) {
#ifdef DIAGNOSTIC
		spurious = 0;
#endif
		return CR_INT_0;
	}

#ifdef DIAGNOSTIC
	if (pending != 0) {
		printf("stray interrupt, mace mask %lx stat %lx\n"
		    "crime mask %lx stat %lx hard %lx (pending %lx caught %lx)\n",
		    bus_space_read_8(&macebus_tag, mace_h, MACE_ISA_INT_MASK),
		    bus_space_read_8(&macebus_tag, mace_h, MACE_ISA_INT_STAT),
		    bus_space_read_8(&crimebus_tag, crime_h, CRIME_INT_MASK),
		    bus_space_read_8(&crimebus_tag, crime_h, CRIME_INT_STAT),
		    bus_space_read_8(&crimebus_tag, crime_h, CRIME_INT_HARD),
		    pending, caught);
		if (++spurious >= 10)
			panic("too many stray interrupts");
	}
#endif

	return 0;  /* Not found here. */
}


/*
 * Macebus auxilary functions run each clock interrupt.
 */
intrmask_t
macebus_aux(intrmask_t hwpend, struct trap_frame *cf)
{
	u_int64_t mask;

	mask = bus_space_read_8(&macebus_tag, mace_h, MACE_ISA_MISC_REG);
	mask |= MACE_ISA_MISC_RLED_OFF | MACE_ISA_MISC_GLED_OFF;

	/* GREEN - Idle */
	/* AMBER - System mode */
	/* RED   - User mode */
	if (cf->sr & SR_KSU_USER) {
		mask &= ~MACE_ISA_MISC_RLED_OFF;
	} else if (curproc == NULL ||
	    curproc == curcpu()->ci_schedstate.spc_idleproc) {
		mask &= ~MACE_ISA_MISC_GLED_OFF;	
	} else {
		mask &= ~(MACE_ISA_MISC_RLED_OFF | MACE_ISA_MISC_GLED_OFF);
	}
	bus_space_write_8(&macebus_tag, mace_h, MACE_ISA_MISC_REG, mask);

	if (maceticks++ > 100*5) {
		maceticks = 0;
	}

	return 0; /* Real clock int handler registers. */
}
