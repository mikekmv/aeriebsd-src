/*
 * Copyright (c) 2008 Michael Shalayeff
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/queue.h>
#include <sys/mutex.h>
#include <sys/buf.h>
#include <sys/disk.h>
#include <sys/disklabel.h>

#include <scsi/scsi_all.h>
#include <scsi/scsi_disk.h>
#include <scsi/scsiconf.h>
#include <scsi/sdvar.h>

#include <uvm/uvm.h>

/* arbitrary numbers */
#define	HIMEM_MAXCMDS	256	/* each one is a page */

/* derived from page table structure */
#define	HIMEM_OFFSET	((sizeof(struct hibuf) + 7) / 8)
#define	HIMEM_MAXSEGS	(512 - HIMEM_OFFSET - 2)
#define	HIMEM_MAXPHYS	(HIMEM_MAXSEGS * PAGE_SIZE)

#define	HIMEM_PDE	(8)
#define	HIMEM_VA	(HIMEM_PDE << 21)
#define	HIMEM_LOW	(HIMEM_VA + (PAGE_SIZE * HIMEM_OFFSET))
#define	HIMEM_HIGH	(HIMEM_VA + (PAGE_SIZE * 512))
#define	PDE_MASK	((512 * (PAGE_SIZE / DEV_BSIZE)) - 1)

void himem_zefix(u_int64_t *, void *, void *, u_int);	/* locore.s */

struct hibuf {
	TAILQ_ENTRY(hibuf) hb_list;
	paddr_t hb_pa;
	struct scsi_xfer *hb_xs;
	void *hb_src, *hb_dst;
	u_int hb_bno, hb_len;
	int hb_flags;
#define	HIMEM_WAKE	0x0001
};

struct himem_softc {
	struct device sc_dev;
	struct scsi_link sc_link;

	int sc_flags;
#define	HIMEM_RDONLY	0x0001
#define	HIMEM_DISKLABEL	0x0002
	int sc_size;	/* blocks */

	struct proc *sc_kthread;
	u_int64_t *sc_pdir;
	paddr_t sc_paddr;
	struct mutex sc_inmtx;
	struct mutex sc_freemtx;
	TAILQ_HEAD(hibuf_head, hibuf) sc_free, sc_in;
};

int  himem_scsi_cmd(struct scsi_xfer *);
int  himem_scsi_ioctl(struct scsi_link *, u_long, caddr_t, int, struct proc *);
void himeminphys(struct buf *bp);

struct scsi_adapter himem_switch = {
	himem_scsi_cmd, himeminphys, 0, 0, himem_scsi_ioctl
};

struct scsi_device himem_dev = {
	NULL, NULL, NULL, NULL
};

int  himem_match(struct device *, void *, void *);
void himem_attach(struct device *, struct device *, void *);

struct cfattach himem_ca = {
	sizeof(struct himem_softc), himem_match, himem_attach
};

struct cfdriver himem_cd = {
	NULL, "himem", DV_DULL
};

void himem(void *);
void himem_create(void *);

int
himem_match(struct device *parent, void *match, void *aux)
{
	extern u_int64_t avail_end2;	/* from machdep.c */
	struct cfdata *cf = match;

	if (cf->cf_unit)
		return 0;

	/* if no PAE or too little memory then screw it */
	if (!(cpu_feature & CPUID_PAE) || avail_end2 < 0x100000000ULL ||
	    (avail_end2 - 0x100000000ULL) < 4 * HIMEM_MAXCMDS * PAGE_SIZE)
		return 0;

	return 1;
}

void
himem_attach(struct device *parent, struct device *self, void *aux)
{
	extern u_int64_t avail_end2;
	struct himem_softc *sc = (struct himem_softc *)self;
	struct disklabel *lp;
	struct hibuf *bp;
	paddr_t pa;
	vaddr_t va;
	int i, pdsize;
	
	TAILQ_INIT(&sc->sc_in);
	TAILQ_INIT(&sc->sc_free);
	mtx_init(&sc->sc_inmtx, IPL_HIGH);
	mtx_init(&sc->sc_freemtx, IPL_HIGH);

	pdsize = 4 * PAGE_SIZE;
	sc->sc_pdir = (u_int64_t *)uvm_km_alloc1(kernel_map, pdsize,
	    PAGE_SIZE, 1);
	if (!sc->sc_pdir) {
		printf(": cannot allocate page index\n");
		return;
	}

#define	vatopa(va) pmap_extract(pmap_kernel(), (vaddr_t)va, &pa)
	/* we do know like for sure there ain't no like user space */
	vatopa((vaddr_t)sc->sc_pdir + 0 * PAGE_SIZE);	sc->sc_paddr = pa;
	sc->sc_pdir[0] = pa | PG_V;
	vatopa((vaddr_t)sc->sc_pdir + 1 * PAGE_SIZE);
	sc->sc_pdir[1] = pa | PG_V;
	vatopa((vaddr_t)sc->sc_pdir + 2 * PAGE_SIZE);
	sc->sc_pdir[2] = pa | PG_V;
	vatopa((vaddr_t)sc->sc_pdir + 3 * PAGE_SIZE);
	sc->sc_pdir[3] = pa | PG_V;

	/* 8M of kernel code is ment to be enough for everbody */
	sc->sc_pdir[(KERNBASE >> 21) + 0] = 0x000000 |
	    PG_KW | PG_M | PG_U | PG_V | PG_PS;
	sc->sc_pdir[(KERNBASE >> 21) + 1] = 0x200000 |
	    PG_KW | PG_M | PG_U | PG_V | PG_PS;
	sc->sc_pdir[(KERNBASE >> 21) + 2] = 0x400000 |
	    PG_KW | PG_M | PG_U | PG_V | PG_PS;
	sc->sc_pdir[(KERNBASE >> 21) + 3] = 0x600000 |
	    PG_KW | PG_M | PG_U | PG_V | PG_PS;

	bp = (struct hibuf *)uvm_km_alloc1(kernel_map,
	    HIMEM_MAXCMDS * PAGE_SIZE, PAGE_SIZE, 1);
	if (!bp) {
		uvm_km_free(kmem_map, (vaddr_t)sc->sc_pdir, pdsize);
		printf(": no memory for buffers\n");
		return;
	}

	/* each buf is a page table and hibuf in one! */
	for (i = HIMEM_MAXCMDS, va = (vaddr_t)bp; i--; va += PAGE_SIZE) {
		bp = (struct hibuf *)va;
		TAILQ_INSERT_TAIL(&sc->sc_free, bp, hb_list);
		pmap_extract(pmap_kernel(), va, &bp->hb_pa);
	}

	sc->sc_size = (avail_end2 - 0x100000000ULL) >> DEV_BSHIFT;
	printf(": size %uMB\n", sc->sc_size / 2048);

	/* set a fake disklabel */
	lp = (void *)uvm_km_zalloc(kernel_map, round_page(sizeof(*lp)));
	if (lp) {
		lp->d_secsize = DEV_BSIZE;
		lp->d_ntracks = 255;
		lp->d_nsectors = 63;
		lp->d_secpercyl = lp->d_ntracks * lp->d_nsectors;
		lp->d_ncylinders = sc->sc_size / lp->d_secpercyl;
		lp->d_type = DTYPE_SCSI;
		strncpy(lp->d_typename, "SCSI disk", sizeof(lp->d_typename));
		strncpy(lp->d_packname, "HIMEM drive", sizeof(lp->d_packname));
		DL_SETDSIZE(lp, sc->sc_size);
		lp->d_rpm = 32768;
		lp->d_interleave = 1;
		lp->d_version = 1;
		lp->d_flags = 0;
		lp->d_bbsize = 8192;
		lp->d_sbsize = 65536;
		lp->d_magic = DISKMAGIC;
		lp->d_magic2 = DISKMAGIC;

		lp->d_npartitions = MAXPARTITIONS;
		lp->d_partitions[0].p_fstype = FS_BSDFFS;
		DL_SETPSIZE(&lp->d_partitions[0], sc->sc_size);
		DL_SETPOFFSET(&lp->d_partitions[0], 0);
		lp->d_partitions[0].p_fragblock =
		    DISKLABELV1_FFS_FRAGBLOCK(4096, 4);
		DL_SETPSIZE(&lp->d_partitions[RAW_PART], sc->sc_size);
		DL_SETPOFFSET(&lp->d_partitions[RAW_PART], 0);

		lp->d_checksum = dkcksum(lp);

		/* write out the label */
		vatopa(lp);
		sc->sc_pdir[HIMEM_PDE] = PG_KW | PG_U | PG_M | PG_V | PG_PS |
		    (pa & ~(0x200000 - 1));
		sc->sc_pdir[HIMEM_PDE+1] = PG_KW | PG_U | PG_M | PG_V | PG_PS |
		    0x100000000ULL;

		himem_zefix(sc->sc_pdir,
		    (char *)HIMEM_VA + (pa & (0x200000 - 1)),
		    (char *)HIMEM_HIGH + LABELSECTOR * DEV_BSIZE, DEV_BSIZE);

		sc->sc_pdir[HIMEM_PDE] = 0;
		sc->sc_pdir[HIMEM_PDE+1] = 0;
		uvm_km_free(kernel_map, (vaddr_t)lp, round_page(sizeof(*lp)));
	}
#undef vatopa

	kthread_create_deferred(himem_create, sc);
}

void
himem_create(void *v)
{
	struct himem_softc *sc = v;

	kthread_create(himem, sc, &sc->sc_kthread, "himem.sys");
}

int
himem_scsi_cmd(struct scsi_xfer *xs)
{
	struct scsi_link *link = xs->sc_link;
	struct himem_softc *sc = link->adapter_softc;
	struct scsi_read_cap_data rcd;
	struct scsi_inquiry_data inq;
	struct hibuf *bp;
	u_int64_t *ptes;
	paddr_t pa;
	vaddr_t va, eva;
	u_int bno, bcnt;
	int s, res;

#if 0
	/* we do not want to be double-buffered */
	if (xs->bp && (xs->bp->b_flags & B_DELWRI))
		xs->bp->b_flags |= B_NOCACHE;
#endif

	s = splbio();
	if (link->target > 0 || !sc->sc_size || link->lun != 0) {
		bzero(&xs->sense, sizeof(xs->sense));
		xs->sense.error_code = SSD_ERRCODE_VALID | 0x70;
		xs->sense.flags = SKEY_ILLEGAL_REQUEST;
		xs->sense.add_sense_code = 0x20; /* illcmd, 0x24 illfield */
		xs->error = XS_SENSE;
		scsi_done(xs);
		splx(s);
		return (COMPLETE);
	}

	xs->error = XS_NOERROR;
	switch (xs->cmd->opcode) {
	default:
		xs->sense.error_code = SSD_ERRCODE_VALID | 0x70;
		xs->sense.flags = SKEY_ILLEGAL_REQUEST;
		xs->sense.add_sense_code = 0x20; /* illcmd, 0x24 illfield */
		xs->error = XS_SENSE;
		scsi_done(xs);
		splx(s);
		return (COMPLETE);

	case TEST_UNIT_READY:
	case START_STOP:
	case PREVENT_ALLOW:
		splx(s);
		return COMPLETE;

	case INQUIRY:
		bzero(&inq, sizeof(inq));
		inq.device = T_DIRECT;
		inq.dev_qual2 = 0;
		inq.version = 2;
		inq.response_format = 2;
		inq.additional_length = 32;
		strlcpy(inq.vendor, "McIkye ", sizeof(inq.vendor));
		strlcpy(inq.product, "HIMEM drive    ", sizeof(inq.product));
		strlcpy(inq.revision, "1.1", sizeof(inq.revision));
		bcopy(&inq, xs->data, MIN(sizeof(inq), xs->datalen));
		scsi_done(xs);
		splx(s);
		return COMPLETE;

	case READ_CAPACITY:
		bzero(&rcd, sizeof(rcd));
		_lto4b(sc->sc_size - 1, rcd.addr);
		_lto4b(DEV_BSIZE, rcd.length);
		bcopy(&rcd, xs->data, MIN(sizeof(rcd), xs->datalen));
		scsi_done(xs);
		splx(s);
		return COMPLETE;

	case SYNCHRONIZE_CACHE:
		mtx_enter(&sc->sc_inmtx);
		if (!TAILQ_EMPTY(&sc->sc_in)) {
			wakeup(&sc->sc_in);
			bp = TAILQ_LAST(&sc->sc_in, hibuf_head);
			bp->hb_flags |= HIMEM_WAKE;
			msleep(bp, &sc->sc_inmtx, PRIBIO, "himem.sync", 0);
		}
		mtx_leave(&sc->sc_inmtx);
		scsi_done(xs);
		splx(s);
		return COMPLETE;

	case WRITE_COMMAND:
	case WRITE_BIG:
		if (sc->sc_flags & HIMEM_RDONLY) {
			xs->sense.error_code = SSD_ERRCODE_VALID | 0x70;
			xs->sense.flags = SKEY_WRITE_PROTECT;
			xs->sense.add_sense_code = 0;
			xs->error = XS_SENSE;
			scsi_done(xs);
			splx(s);
			return COMPLETE;
		}
	case READ_COMMAND:
	case READ_BIG:
		if (xs->cmdlen == 6) {
			struct scsi_rw *rw = (struct scsi_rw *)xs->cmd;
			bno = _3btol(rw->addr) & (SRW_TOPADDR << 16 | 0xffff);
			bcnt = rw->length ? rw->length : 0x100;
		} else {
			struct scsi_rw_big *rwb = (struct scsi_rw_big *)xs->cmd;
			bno = _4btol(rwb->addr);
			bcnt = _2btol(rwb->length);
		}

		if (bno >= sc->sc_size || bno + bcnt > sc->sc_size) {
			xs->error = XS_DRIVER_STUFFUP;
			scsi_done(xs);
			splx(s);
			return COMPLETE;
		}

		mtx_enter(&sc->sc_freemtx);
		bp = TAILQ_FIRST(&sc->sc_free);
		TAILQ_REMOVE(&sc->sc_free, bp, hb_list);
		mtx_leave(&sc->sc_freemtx);
		splx(s);

		bp->hb_xs = xs;
		res = (vaddr_t)xs->data & PAGE_MASK;
		if (xs->cmd->opcode == READ_COMMAND ||
		    xs->cmd->opcode == READ_BIG) {
			bp->hb_dst = (char *)HIMEM_LOW + res;
			bp->hb_src = (char *)HIMEM_HIGH +
			    ((bno & PDE_MASK) << DEV_BSHIFT);
		} else {
			bp->hb_src = (char *)HIMEM_LOW + res;
			bp->hb_dst = (char *)HIMEM_HIGH +
			    ((bno & PDE_MASK) << DEV_BSHIFT);
		}

		bp->hb_len = xs->datalen;
		bp->hb_bno = bno;
		bp->hb_flags = 0;

		ptes = (u_int64_t *)bp + HIMEM_OFFSET;
		for (va = (vaddr_t)xs->data - res,
		    eva = (vaddr_t)xs->data + xs->datalen + PAGE_SIZE - 1;
		    va < eva; va += PAGE_SIZE) {
			pmap_extract(pmap_kernel(), va, &pa);
			*ptes++ = pa | PG_KW | PG_U | PG_M | PG_V;
		}

		if (xs->flags & SCSI_POLL)
			bp->hb_flags |= HIMEM_WAKE;

		mtx_enter(&sc->sc_inmtx);
		TAILQ_INSERT_TAIL(&sc->sc_in, bp, hb_list);
		mtx_leave(&sc->sc_inmtx);
		wakeup(&sc->sc_in);
		if (xs->flags & SCSI_POLL) {
			tsleep(bp, PRIBIO, "himem.poll", 0);
			return COMPLETE;
		} else
			return SUCCESSFULLY_QUEUED;
	}
}

int
himem_scsi_ioctl(struct scsi_link *link, u_long cmd, caddr_t addr, int flag,
    struct proc *p)
{
	/* struct himem_softc *sc = link->adapter_softc; */

	/* TODO implement set-rdonly */
	return ENOTTY;
}

void
himeminphys(struct buf *bp)
{
	if (bp->b_bcount > HIMEM_MAXPHYS)
		bp->b_bcount = HIMEM_MAXPHYS;
	minphys(bp);
}

void
himem(void *v)
{
	struct scsibus_attach_args saa;
	struct himem_softc *sc = v;
	struct scsi_xfer *xs;
	struct hibuf *bp;
	int s, wake;

	sc->sc_link.device = &himem_dev;
	sc->sc_link.device_softc = sc;
	sc->sc_link.adapter = &himem_switch;
	sc->sc_link.adapter_softc = sc;
	sc->sc_link.adapter_target = 1;
	sc->sc_link.adapter_buswidth = 1;
	sc->sc_link.openings = HIMEM_MAXCMDS;
	bzero(&saa, sizeof(saa));
	saa.saa_sc_link = &sc->sc_link;
	config_found(&sc->sc_dev, &saa, scsiprint);

	KERNEL_PROC_UNLOCK(curproc);

	for (;;) {
		mtx_enter(&sc->sc_inmtx);
		while (TAILQ_EMPTY(&sc->sc_in))
			msleep(&sc->sc_in, &sc->sc_inmtx, PRIBIO, "himem.sys", 0);

		bp = TAILQ_FIRST(&sc->sc_in);
		TAILQ_REMOVE(&sc->sc_in, bp, hb_list);
		mtx_leave(&sc->sc_inmtx);

		sc->sc_pdir[HIMEM_PDE] = bp->hb_pa | PG_KW | PG_U | PG_M | PG_V;
		sc->sc_pdir[HIMEM_PDE+1] = PG_KW | PG_U | PG_M | PG_V | PG_PS |
		    (0x100000000ULL +
		     ((uint64_t)(bp->hb_bno & ~PDE_MASK) << DEV_BSHIFT));
		sc->sc_pdir[HIMEM_PDE+2] = sc->sc_pdir[HIMEM_PDE+1] + 0x200000;

		himem_zefix(sc->sc_pdir, bp->hb_src, bp->hb_dst, bp->hb_len);

		sc->sc_pdir[HIMEM_PDE] = 0;
		sc->sc_pdir[HIMEM_PDE+1] = 0;

		xs = bp->hb_xs;
		xs->resid -= bp->hb_len;
		xs->flags |= ITSDONE;
		wake = bp->hb_flags & HIMEM_WAKE;
		mtx_enter(&sc->sc_freemtx);
		TAILQ_INSERT_HEAD(&sc->sc_free, bp, hb_list);
		mtx_leave(&sc->sc_freemtx);

		KERNEL_PROC_LOCK(curproc);
		s = splbio();
		scsi_done(xs);
		splx(s);
		if (wake)
			wakeup(bp);
		KERNEL_PROC_UNLOCK(curproc);
	}
}
