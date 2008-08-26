/*
 * Copyright (c) 2005 Michael Shalayeff
 * Copyright (c) 2007 Mark Kettenis
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
#include <sys/device.h>
#include <sys/extent.h>
#include <sys/malloc.h>
#include <sys/reboot.h>
#include <sys/tree.h>

#include <uvm/uvm_extern.h>

#include <machine/iomod.h>
#include <machine/autoconf.h>
#include <machine/cpufunc.h>

#include <hppa/dev/cpudevs.h>

struct pluto_regs {
	u_int32_t	rid;
	u_int32_t	pad0000;
	u_int32_t	ioc_ctrl;
	u_int32_t	pad0008;
	u_int8_t	resv1[0x0300 - 0x0010];
	u_int64_t	lmmio_direct0_base;
	u_int64_t	lmmio_direct0_mask;
	u_int64_t	lmmio_direct0_route;
	u_int64_t	lmmio_direct1_base;
	u_int64_t	lmmio_direct1_mask;
	u_int64_t	lmmio_direct1_route;
	u_int64_t	lmmio_direct2_base;
	u_int64_t	lmmio_direct2_mask;
	u_int64_t	lmmio_direct2_route;
	u_int64_t	lmmio_direct3_base;
	u_int64_t	lmmio_direct3_mask;
	u_int64_t	lmmio_direct3_route;
	u_int64_t	lmmio_dist_base;
	u_int64_t	lmmio_dist_mask;
	u_int64_t	lmmio_dist_route;
	u_int64_t	gmmio_dist_base;
	u_int64_t	gmmio_dist_mask;
	u_int64_t	gmmio_dist_route;
	u_int64_t	ios_dist_base;
	u_int64_t	ios_dist_mask;
	u_int64_t	ios_dist_route;
	u_int8_t	resv2[0x03c0 - 0x03a8];
	u_int64_t	ios_direct_base;
	u_int64_t	ios_direct_mask;
	u_int64_t	ios_direct_route;
	u_int8_t	resv3[0x22000 - 0x03d8];
	u_int64_t	func_id;
	u_int64_t	func_class;
	u_int8_t	resv4[0x22040 - 0x22010];
	u_int64_t	rope_config;
	u_int8_t	resv5[0x22050 - 0x22048];
	u_int64_t	rope_debug;
	u_int8_t	resv6[0x22200 - 0x22058];
	u_int64_t	rope0_control;
	u_int64_t	rope1_control;
	u_int64_t	rope2_control;
	u_int64_t	rope3_control;
	u_int64_t	rope4_control;
	u_int64_t	rope5_control;
	u_int64_t	rope6_control;
	u_int64_t	rope7_control;
	u_int8_t	resv7[0x22300 - 0x22240];
	u_int32_t	tlb_ibase;
	u_int32_t	pad22300;
	u_int32_t	tlb_imask;
	u_int32_t	pad22308;
	u_int32_t	tlb_pcom;
	u_int32_t	pad22310;
	u_int32_t	tlb_tcnfg;
	u_int32_t	pad22318;
	u_int64_t	tlb_pdir_base;
};

#define PLUTO_IOC_CTRL_TE	0x0001	/* TOC Enable */
#define PLUTO_IOC_CTRL_CE	0x0002	/* Coalesce Enable */
#define PLUTO_IOC_CTRL_DE	0x0004	/* Dillon Enable */
#define PLUTO_IOC_CTRL_IE	0x0008	/* IOS Enable */
#define PLUTO_IOC_CTRL_OS	0x0010	/* Outbound Synchronous */
#define PLUTO_IOC_CTRL_IS	0x0020	/* Inbound Synchronous */
#define PLUTO_IOC_CTRL_RC	0x0040	/* Read Current Enable */
#define PLUTO_IOC_CTRL_L0	0x0080	/* 0-length Read Enable */
#define PLUTO_IOC_CTRL_RM	0x0100	/* Real Mode */
#define PLUTO_IOC_CTRL_NC	0x0200	/* Non-coherent Mode */
#define PLUTO_IOC_CTRL_ID	0x0400	/* Interrupt Disable */
#define PLUTO_IOC_CTRL_D4	0x0800	/* Disable 4-byte Coalescing */
#define PLUTO_IOC_CTRL_CC	0x1000	/* Increase Coalescing counter value */
#define PLUTO_IOC_CTRL_DD	0x2000	/* Disable distr. range coalescing */
#define PLUTO_IOC_CTRL_DC	0x4000	/* Disable the coalescing counter */

#define IOTTE_V		0x8000000000000000LL	/* Entry valid */
#define IOTTE_PAMASK	0x000000fffffff000LL
#define IOTTE_CI	0x00000000000000ffLL	/* Coherent index */

struct pluto_softc {
	struct device sc_dv;

	bus_dma_tag_t sc_dmat;
	struct pluto_regs volatile *sc_regs;
	u_int64_t *sc_pdir;

	char sc_dvmamapname[20];
	struct extent *sc_dvmamap;
	struct hppa64_bus_dma_tag sc_dmatag;
};

/*
 * per-map DVMA page table
 */
struct pluto_page_entry {
	SPLAY_ENTRY(pluto_page_entry) ipe_node;
	paddr_t	ipe_pa;
	vaddr_t	ipe_va;
	bus_addr_t ipe_dva;
};

struct pluto_page_map {
	SPLAY_HEAD(pluto_page_tree, pluto_page_entry) ipm_tree;
	int ipm_maxpage;	/* Size of allocated page map */
	int ipm_pagecnt;	/* Number of entries in use */
	struct pluto_page_entry	ipm_map[1];
};

/*
 * per-map IOMMU state
 */
struct pluto_map_state {
	struct pluto_softc *ims_sc;
	bus_addr_t ims_dvmastart;
	bus_size_t ims_dvmasize;
	struct pluto_page_map ims_map;	/* map must be last (array at end) */
};

int	pluto_match(struct device *, void *, void *);
void	pluto_attach(struct device *, struct device *, void *);

struct cfattach plut_ca = {
	sizeof(struct pluto_softc), pluto_match, pluto_attach
};

struct cfdriver plut_cd = {
	NULL, "plut", DV_DULL
};

int	pluto_dvmamap_create(void *, bus_size_t, int, bus_size_t, bus_size_t,
	    int, bus_dmamap_t *);
void	pluto_dvmamap_destroy(void *, bus_dmamap_t);
int	pluto_dvmamap_load(void *, bus_dmamap_t, void *, bus_size_t,
	    struct proc *, int);
int	pluto_dvmamap_load_mbuf(void *, bus_dmamap_t, struct mbuf *, int);
int	pluto_dvmamap_load_uio(void *, bus_dmamap_t, struct uio *, int);
int	pluto_dvmamap_load_raw(void *, bus_dmamap_t, bus_dma_segment_t *,
	    int, bus_size_t, int);
void	pluto_dvmamap_unload(void *, bus_dmamap_t);
void	pluto_dvmamap_sync(void *, bus_dmamap_t, bus_addr_t, bus_size_t, int);
int	pluto_dvmamem_alloc(void *, bus_size_t, bus_size_t, bus_size_t,
	    bus_dma_segment_t *, int, int *, int);
void	pluto_dvmamem_free(void *, bus_dma_segment_t *, int);
int	pluto_dvmamem_map(void *, bus_dma_segment_t *, int, size_t,
	    caddr_t *, int);
void	pluto_dvmamem_unmap(void *, caddr_t, size_t);
paddr_t	pluto_dvmamem_mmap(void *, bus_dma_segment_t *, int, off_t, int, int);

void	pluto_enter(struct pluto_softc *, bus_addr_t, paddr_t, vaddr_t, int);
void	pluto_remove(struct pluto_softc *, bus_addr_t);

struct pluto_map_state *pluto_iomap_create(int);
void	pluto_iomap_destroy(struct pluto_map_state *);
int	pluto_iomap_insert_page(struct pluto_map_state *, vaddr_t, paddr_t);
bus_addr_t pluto_iomap_translate(struct pluto_map_state *, paddr_t);
void	pluto_iomap_clear_pages(struct pluto_map_state *);

const struct hppa64_bus_dma_tag pluto_dmat = {
	NULL,
	pluto_dvmamap_create, pluto_dvmamap_destroy,
	pluto_dvmamap_load, pluto_dvmamap_load_mbuf,
	pluto_dvmamap_load_uio, pluto_dvmamap_load_raw,
	pluto_dvmamap_unload, pluto_dvmamap_sync,

	pluto_dvmamem_alloc, pluto_dvmamem_free, pluto_dvmamem_map,
	pluto_dvmamem_unmap, pluto_dvmamem_mmap
};

int
pluto_match(struct device *parent, void *cfdata, void *aux)   
{
	struct confargs *ca = aux;

	/* Astro is a U-Turn variant. */
	if (ca->ca_type.iodc_type != HPPA_TYPE_IOA ||
	    ca->ca_type.iodc_sv_model != HPPA_IOA_UTURN)
		return 0;

	if (ca->ca_type.iodc_model == 0x58 &&
	    ca->ca_type.iodc_revision >= 0x20)
		return 1;

	return 0;
}

void
pluto_attach(struct device *parent, struct device *self, void *aux)
{
	struct confargs *ca = aux, nca;
	struct pluto_softc *sc = (struct pluto_softc *)self;
	volatile struct pluto_regs *r;
	bus_space_handle_t ioh;
	u_int32_t ver, ioc_ctrl;
	psize_t size;
	vaddr_t va;
	paddr_t pa;
	struct vm_page *m;
	struct pglist mlist;
	int iova_bits;
	char buf[16];

	sc->sc_dmat = ca->ca_dmatag;
	if (bus_space_map(ca->ca_iot, ca->ca_hpa, sizeof(struct pluto_regs),
	    0, &ioh)) {
		printf(": can't map IO space\n");
		return;
	}
	sc->sc_regs = r = bus_space_vaddr(ca->ca_iot, ioh);

	ver = letoh32(r->rid);
	switch ((ca->ca_type.iodc_model << 4) |
	    (ca->ca_type.iodc_revision >> 4)) {
	case 0x582:
	case 0x780:
		snprintf(buf, sizeof(buf), "Astro rev %d.%d",
		    (ver & 7) + 1, (ver >> 3) & 3);
		break;

	case 0x803:
	case 0x781:
		snprintf(buf, sizeof(buf), "Ike rev %d", ver & 0xff);
		break;

	case 0x804:
	case 0x782:
		snprintf(buf, sizeof(buf), "Reo rev %d", ver & 0xff);
		break;

	case 0x880:
	case 0x784:
		snprintf(buf, sizeof(buf), "Pluto rev %d.%d",
		    (ver >> 4) & 0xf, ver & 0xf);
		break;

	default:
		snprintf(buf, sizeof(buf), "Fluffy rev 0x%x", ver);
		break;
	}
	printf(": %s\n", buf);

	ioc_ctrl = letoh32(r->ioc_ctrl);
	ioc_ctrl &= ~PLUTO_IOC_CTRL_CE;
	ioc_ctrl &= ~PLUTO_IOC_CTRL_RM;
	ioc_ctrl &= ~PLUTO_IOC_CTRL_NC;
	r->ioc_ctrl = htole32(ioc_ctrl);

	/*
	 * Setup the pluto.
	 */

	/* XXX This gives us 256MB of iova space. */
	iova_bits = 28;

	r->tlb_ibase = htole32(0);
	r->tlb_imask = htole32(0xffffffff << iova_bits);

	/* Page size is 4K. */
	r->tlb_tcnfg = htole32(0);

	/* Flush TLB. */
	r->tlb_pcom = htole32(31);

	/*
	 * Allocate memory for I/O pagetables.  They need to be physically
	 * contiguous.
	 */

	size = (1 << (iova_bits - PAGE_SHIFT)) * sizeof(u_int64_t);
	TAILQ_INIT(&mlist);
	if (uvm_pglistalloc(size, 0, -1, PAGE_SIZE, 0, &mlist, 1, 0) != 0)
		panic("plutottach: no memory");

	va = uvm_km_valloc(kernel_map, size);
	if (va == 0)
		panic("plutoattach: no memory");
	sc->sc_pdir = (u_int64_t *)va;

	m = TAILQ_FIRST(&mlist);
	r->tlb_pdir_base = htole64(VM_PAGE_TO_PHYS(m));

	/* Map the pages. */
	for (; m != NULL; m = TAILQ_NEXT(m, pageq)) {
		pa = VM_PAGE_TO_PHYS(m);
		pmap_enter(pmap_kernel(), va, pa,
		    VM_PROT_READ|VM_PROT_WRITE, PMAP_WIRED);
		va += PAGE_SIZE;
	}
	pmap_update(pmap_kernel());
	memset(sc->sc_pdir, 0, size);

	/*
	 * The PDC might have set up some devices to do DMA.  It will do
	 * this for the onboard USB controller if an USB keyboard is used
	 * for console input.  In that case, bad things will happen if we
	 * enable iova space.  So reset the PDC devices before we do that.
	 * Don't do this if we're using a serial console though, since it
	 * will stop working if we do.  This is fine since the serial port
	 * doesn't do DMA.
	 */
	if (PAGE0->mem_cons.pz_class != PCL_DUPLEX)
		pdc_call((iodcio_t)pdc, 0, PDC_IO, PDC_IO_RESET_DEVICES);

	/* Enable iova space. */
	r->tlb_ibase = htole32(1);

        /*
         * Now all the hardware's working we need to allocate a dvma map.
         */
	snprintf(sc->sc_dvmamapname, sizeof(sc->sc_dvmamapname),
	    "%s_dvma", sc->sc_dv.dv_xname);
        sc->sc_dvmamap = extent_create(sc->sc_dvmamapname, 0, (1 << iova_bits),
            M_DEVBUF, 0, 0, EX_NOWAIT);

	sc->sc_dmatag = pluto_dmat;
	sc->sc_dmatag._cookie = sc;

	nca = *ca;	/* clone from us */
	nca.ca_dmatag = &sc->sc_dmatag;
	pdc_scan(self, &nca);
}

int
pluto_dvmamap_create(void *v, bus_size_t size, int nsegments,
    bus_size_t maxsegsz, bus_size_t boundary, int flags, bus_dmamap_t *dmamap)
{
	struct pluto_softc *sc = v;
	bus_dmamap_t map;
	struct pluto_map_state *ims;
	int error;

	error = bus_dmamap_create(sc->sc_dmat, size, nsegments, maxsegsz,
	    boundary, flags, &map);
	if (error)
		return (error);

	ims = pluto_iomap_create(atop(round_page(size)));
	if (ims == NULL) {
		bus_dmamap_destroy(sc->sc_dmat, map);
		return (ENOMEM);
	}

	ims->ims_sc = sc;
	map->_dm_cookie = ims;
	*dmamap = map;

	return (0);
}

void
pluto_dvmamap_destroy(void *v, bus_dmamap_t map)
{
	struct pluto_softc *sc = v;

	/*
	 * The specification (man page) requires a loaded
	 * map to be unloaded before it is destroyed.
	 */
	if (map->dm_nsegs)
		pluto_dvmamap_unload(sc, map);

        if (map->_dm_cookie)
                pluto_iomap_destroy(map->_dm_cookie);
	map->_dm_cookie = NULL;

	bus_dmamap_destroy(sc->sc_dmat, map);
}

int
pluto_iomap_load_map(struct pluto_softc *sc, bus_dmamap_t map, int flags)
{
	struct pluto_map_state *ims = map->_dm_cookie;
	struct pluto_page_map *ipm = &ims->ims_map;
	struct pluto_page_entry *e;
	int err, seg, s;
	paddr_t pa, paend;
	vaddr_t va;
	bus_size_t sgsize;
	bus_size_t align, boundary;
	u_long dvmaddr;
	bus_addr_t dva;
	int i;

	/* XXX */
	boundary = map->_dm_boundary;
	align = PAGE_SIZE;

	pluto_iomap_clear_pages(ims);

	for (seg = 0; seg < map->dm_nsegs; seg++) {
		struct hppa64_bus_dma_segment *ds = &map->dm_segs[seg];

		paend = round_page(ds->ds_addr + ds->ds_len);
		for (pa = trunc_page(ds->ds_addr), va = trunc_page(ds->_ds_va);
		     pa < paend; pa += PAGE_SIZE, va += PAGE_SIZE) {
			err = pluto_iomap_insert_page(ims, va, pa);
			if (err) {
                               printf("iomap insert error: %d for "
                                    "va 0x%lx pa 0x%lx\n", err, va, pa);
				bus_dmamap_unload(sc->sc_dmat, map);
				pluto_iomap_clear_pages(ims);
			}
		}
	}

	sgsize = ims->ims_map.ipm_pagecnt * PAGE_SIZE;
	s = splhigh();
	err = extent_alloc(sc->sc_dvmamap, sgsize, align, 0, boundary,
	    EX_NOWAIT | EX_BOUNDZERO, &dvmaddr);
	splx(s);
	if (err)
		return (err);

	ims->ims_dvmastart = dvmaddr;
	ims->ims_dvmasize = sgsize;

	dva = dvmaddr;
	for (i = 0, e = ipm->ipm_map; i < ipm->ipm_pagecnt; ++i, ++e) {
		e->ipe_dva = dva;
		pluto_enter(sc, e->ipe_dva, e->ipe_pa, e->ipe_va, flags);
		dva += PAGE_SIZE;
	}

	for (seg = 0; seg < map->dm_nsegs; seg++) {
		struct hppa64_bus_dma_segment *ds = &map->dm_segs[seg];
		ds->ds_addr = pluto_iomap_translate(ims, ds->ds_addr);
	}

	return (0);
}

int
pluto_dvmamap_load(void *v, bus_dmamap_t map, void *addr, bus_size_t size,
    struct proc *p, int flags)
{
	struct pluto_softc *sc = v;
	int err;

	err = bus_dmamap_load(sc->sc_dmat, map, addr, size, p, flags);
	if (err)
		return (err);

	return pluto_iomap_load_map(sc, map, flags);
}

int
pluto_dvmamap_load_mbuf(void *v, bus_dmamap_t map, struct mbuf *m, int flags)
{
	struct pluto_softc *sc = v;
	int err;

	err = bus_dmamap_load_mbuf(sc->sc_dmat, map, m, flags);
	if (err)
		return (err);

	return pluto_iomap_load_map(sc, map, flags);
}

int
pluto_dvmamap_load_uio(void *v, bus_dmamap_t map, struct uio *uio, int flags)
{
	struct pluto_softc *sc = v;

	printf("load_uio\n");

	return (bus_dmamap_load_uio(sc->sc_dmat, map, uio, flags));
}

int
pluto_dvmamap_load_raw(void *v, bus_dmamap_t map, bus_dma_segment_t *segs,
    int nsegs, bus_size_t size, int flags)
{
	struct pluto_softc *sc = v;

	printf("load_raw\n");

	return (bus_dmamap_load_raw(sc->sc_dmat, map, segs, nsegs, size, flags));
}

void
pluto_dvmamap_unload(void *v, bus_dmamap_t map)
{
	struct pluto_softc *sc = v;
	struct pluto_map_state *ims = map->_dm_cookie;
	struct pluto_page_map *ipm = &ims->ims_map;
	struct pluto_page_entry *e;
	int err, i, s;

	/* Remove the IOMMU entries. */
	for (i = 0, e = ipm->ipm_map; i < ipm->ipm_pagecnt; ++i, ++e)
		pluto_remove(sc, e->ipe_dva);

	/* Clear the iomap. */
	pluto_iomap_clear_pages(ims);

	bus_dmamap_unload(sc->sc_dmat, map);

	s = splhigh();
	err = extent_free(sc->sc_dvmamap, ims->ims_dvmastart,
	    ims->ims_dvmasize, EX_NOWAIT);
	ims->ims_dvmastart = 0;
	ims->ims_dvmasize = 0;
	splx(s);
	if (err)
		printf("warning: %ld of DVMA space lost\n", ims->ims_dvmasize);
}

void
pluto_dvmamap_sync(void *v, bus_dmamap_t map, bus_addr_t off,
    bus_size_t len, int ops)
{
	/* Nothing to do; DMA is cache-coherent. */
}

int
pluto_dvmamem_alloc(void *v, bus_size_t size, bus_size_t alignment,
    bus_size_t boundary, bus_dma_segment_t *segs,
    int nsegs, int *rsegs, int flags)
{
	struct pluto_softc *sc = v;

	return (bus_dmamem_alloc(sc->sc_dmat, size, alignment, boundary,
	    segs, nsegs, rsegs, flags));
}

void
pluto_dvmamem_free(void *v, bus_dma_segment_t *segs, int nsegs)
{
	struct pluto_softc *sc = v;

	bus_dmamem_free(sc->sc_dmat, segs, nsegs);
}

int
pluto_dvmamem_map(void *v, bus_dma_segment_t *segs, int nsegs, size_t size,
    caddr_t *kvap, int flags)
{
	struct pluto_softc *sc = v;

	return (bus_dmamem_map(sc->sc_dmat, segs, nsegs, size, kvap, flags));
}

void
pluto_dvmamem_unmap(void *v, caddr_t kva, size_t size)
{
	struct pluto_softc *sc = v;

	bus_dmamem_unmap(sc->sc_dmat, kva, size);
}

paddr_t
pluto_dvmamem_mmap(void *v, bus_dma_segment_t *segs, int nsegs, off_t off,
    int prot, int flags)
{
	struct pluto_softc *sc = v;

	return (bus_dmamem_mmap(sc->sc_dmat, segs, nsegs, off, prot, flags));
}

/*
 * Utility function used by splay tree to order page entries by pa.
 */
static inline int
iomap_compare(struct pluto_page_entry *a, struct pluto_page_entry *b)
{
	return ((a->ipe_pa > b->ipe_pa) ? 1 :
		(a->ipe_pa < b->ipe_pa) ? -1 : 0);
}

SPLAY_PROTOTYPE(pluto_page_tree, pluto_page_entry, ipe_node, iomap_compare);

SPLAY_GENERATE(pluto_page_tree, pluto_page_entry, ipe_node, iomap_compare);

/*
 * Create a new iomap.
 */
struct pluto_map_state *
pluto_iomap_create(int n)
{
	struct pluto_map_state *ims;

	/* Safety for heavily fragmented data, such as mbufs */
	n += 4;
	if (n < 16)
		n = 16;

	ims = malloc(sizeof(*ims) + (n - 1) * sizeof(ims->ims_map.ipm_map[0]),
	    M_DEVBUF, M_NOWAIT | M_ZERO);
	if (ims == NULL)
		return (NULL);

	/* Initialize the map. */
	ims->ims_map.ipm_maxpage = n;
	SPLAY_INIT(&ims->ims_map.ipm_tree);

	return (ims);
}

/*
 * Destroy an iomap.
 */
void
pluto_iomap_destroy(struct pluto_map_state *ims)
{
#ifdef DIAGNOSTIC
	if (ims->ims_map.ipm_pagecnt > 0)
		printf("pluto_iomap_destroy: %d page entries in use\n",
		    ims->ims_map.ipm_pagecnt);
#endif

	free(ims, M_DEVBUF);
}

/*
 * Insert a pa entry in the iomap.
 */
int
pluto_iomap_insert_page(struct pluto_map_state *ims, vaddr_t va, paddr_t pa)
{
	struct pluto_page_map *ipm = &ims->ims_map;
	struct pluto_page_entry *e;

	if (ipm->ipm_pagecnt >= ipm->ipm_maxpage) {
		struct pluto_page_entry ipe;

		ipe.ipe_pa = pa;
		if (SPLAY_FIND(pluto_page_tree, &ipm->ipm_tree, &ipe))
			return (0);

		return (ENOMEM);
	}

	e = &ipm->ipm_map[ipm->ipm_pagecnt];

	e->ipe_pa = pa;
	e->ipe_va = va;
	e->ipe_dva = NULL;

	e = SPLAY_INSERT(pluto_page_tree, &ipm->ipm_tree, e);

	/* Duplicates are okay, but only count them once. */
	if (e)
		return (0);

	++ipm->ipm_pagecnt;

	return (0);
}

/*
 * Translate a physical address (pa) into a DVMA address.
 */
bus_addr_t
pluto_iomap_translate(struct pluto_map_state *ims, paddr_t pa)
{
	struct pluto_page_map *ipm = &ims->ims_map;
	struct pluto_page_entry *e;
	struct pluto_page_entry pe;
	paddr_t offset = pa & PAGE_MASK;

	pe.ipe_pa = trunc_page(pa);

	e = SPLAY_FIND(pluto_page_tree, &ipm->ipm_tree, &pe);

	if (e == NULL) {
		panic("couldn't find pa %lx\n", pa);
		return 0;
	}

	return (e->ipe_dva | offset);
}

/*
 * Clear the iomap table and tree.
 */
void
pluto_iomap_clear_pages(struct pluto_map_state *ims)
{
        ims->ims_map.ipm_pagecnt = 0;
        SPLAY_INIT(&ims->ims_map.ipm_tree);
}

/*
 * Add an entry to the IOMMU table.
 */
void
pluto_enter(struct pluto_softc *sc, bus_addr_t dva, paddr_t pa, vaddr_t va,
    int flags)
{
	volatile u_int64_t *tte_ptr = &sc->sc_pdir[dva >> PAGE_SHIFT];
	u_int64_t tte;
	u_int32_t ci;

#ifdef DEBUG
	printf("pluto_enter dva %lx, pa %lx, va %lx\n", dva, pa, va);
#endif

#ifdef DIAGNOSTIC
	tte = letoh64(*tte_ptr);

	if (tte & IOTTE_V) {
		printf("Overwriting valid tte entry (dva %lx pa %lx "
		    "&tte %p tte %llx)\n", dva, pa, tte_ptr, tte);
		extent_print(sc->sc_dvmamap);
		panic("IOMMU overwrite");
	}
#endif

	mtsp(HPPA_SID_KERNEL, 1);
	__asm volatile("lci 0(%%sr1, %1), %0" : "=r" (ci) : "r" (va));

	tte = (pa & IOTTE_PAMASK) | ((ci >> 12) & IOTTE_CI);
	tte |= IOTTE_V;

	*tte_ptr = htole64(tte);
	__asm volatile("fdc 0(%%sr1, %0)\n\tsync" : : "r" (tte_ptr));
}

/*
 * Remove an entry from the IOMMU table.
 */
void
pluto_remove(struct pluto_softc *sc, bus_addr_t dva)
{
	volatile struct pluto_regs *r = sc->sc_regs;
	u_int64_t *tte_ptr = &sc->sc_pdir[dva >> PAGE_SHIFT];
	u_int64_t tte;

#ifdef DIAGNOSTIC
	if (dva != trunc_page(dva)) {
		printf("pluto_remove: unaligned dva: %lx\n", dva);
		dva = trunc_page(dva);
	}
#endif

	tte = letoh64(*tte_ptr);

#ifdef DIAGNOSTIC
	if ((tte & IOTTE_V) == 0) {
		printf("Removing invalid tte entry (dva %lx &tte %p "
		    "tte %llx)\n", dva, tte_ptr, tte);
		extent_print(sc->sc_dvmamap);
		panic("IOMMU remove overwrite");
	}
#endif

	*tte_ptr = htole64(tte & ~IOTTE_V);

	/* Flush IOMMU. */
	r->tlb_pcom = htole32(dva | PAGE_SHIFT);
}
