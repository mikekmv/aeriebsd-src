/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/tty.h>
#include <sys/conf.h>

/*
 *	Block devices.
 */

#include "vnd.h"
#include "sd.h"
#include "cd.h"
#include "st.h"
#include "wd.h"
bdev_decl(wd);
#include "rd.h"

struct bdevsw	bdevsw[] =
{
	bdev_disk_init(NSD,sd),		/* 0: SCSI disk */
	bdev_swap_init(1,sw),		/* 1: should be here swap pseudo-dev */
	bdev_disk_init(NVND,vnd),	/* 2: vnode disk driver */
	bdev_disk_init(NCD,cd),		/* 3: SCSI CD-ROM */
	bdev_disk_init(NWD,wd),		/* 4: ST506/ESDI/IDE disk */
	bdev_notdef(),			/* 5:  */
	bdev_notdef(),			/* 6:  */
	bdev_notdef(),			/* 7:  */
	bdev_disk_init(NRD,rd),		/* 8: RAM disk (for install) */
	bdev_notdef(),			/* 9:  */
	bdev_tape_init(NST,st),		/* 10: SCSI tape */
	bdev_notdef(),			/* 11:  */
	bdev_notdef(),			/* 12:  */
	bdev_notdef(),			/* 13:  */
	bdev_notdef(),			/* 14:  */
	bdev_notdef(),			/* 15:  */
};

int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

/*
 *	Character devices.
 */

/* open, close, write, ioctl */
#define	cdev_lpt_init(c,n) { \
	dev_init(c,n,open), dev_init(c,n,close), (dev_type_read((*))) enodev, \
	dev_init(c,n,write), dev_init(c,n,ioctl), (dev_type_stop((*))) enodev, \
	0, seltrue, (dev_type_mmap((*))) enodev }

#define mmread mmrw
#define mmwrite mmrw
dev_type_read(mmrw);
cdev_decl(mm);
#include "bio.h"
#include "pty.h"
cdev_decl(fd);
#include "st.h"
#include "bpfilter.h"
#include "tun.h"
#include "com.h"
cdev_decl(com);
#include "lpt.h"
cdev_decl(lpt);
#include "ch.h"
#include "ss.h"
#include "uk.h"
cdev_decl(wd);
#include "audio.h"
#include "video.h"
#ifdef XFS
#include <xfs/nxfs.h>
cdev_decl(xfs_dev);
#endif
#include "ksyms.h"

#include "wsdisplay.h"
#include "wskbd.h"
#include "wsmouse.h"
#include "wsmux.h"
#include "pci.h"
cdev_decl(pci);

#include "inet.h"

#include "pf.h"
#include "systrace.h"

struct cdevsw	cdevsw[] =
{
	cdev_cn_init(1,cn),		/* 0: virtual console */
	cdev_swap_init(1,sw),		/* 1: /dev/drum (swap pseudo-device) */
	cdev_ctty_init(1,ctty),		/* 2: controlling terminal */
	cdev_mm_init(1,mm),		/* 3: /dev/{null,mem,kmem,...} */
	cdev_tty_init(NPTY,pts),	/* 4: pseudo-tty slave */
	cdev_ptc_init(NPTY,ptc),	/* 5: pseudo-tty master */
	cdev_log_init(1,log),		/* 6: /dev/klog */
	cdev_fd_init(1,filedesc),	/* 7: file descriptor pseudo-dev */
	cdev_disk_init(NCD,cd),		/* 8: SCSI CD */
	cdev_disk_init(NSD,sd),		/* 9: SCSI disk */
	cdev_tape_init(NST,st),		/* 10: SCSI tape */
	cdev_disk_init(NVND,vnd),	/* 11: vnode disk */
	cdev_bpf_init(NBPFILTER,bpf),	/* 12: berkeley packet filter */
	cdev_tun_init(NTUN,tun),	/* 13: network tunnel */
	cdev_notdef(),			/* 14 */
	cdev_notdef(),			/* 15: */
	cdev_lpt_init(NLPT,lpt),	/* 16: Parallel printer interface */
	cdev_tty_init(NCOM,com),	/* 17: 16C450 serial interface */
	cdev_disk_init(NWD,wd),		/* 18: ST506/ESDI/IDE disk */
	cdev_notdef(),			/* 19: */
	cdev_notdef(),			/* 20: */
	cdev_notdef(),			/* 21: */
	cdev_disk_init(NRD,rd),		/* 22: ramdisk device */
	cdev_notdef(),			/* 23: */
	cdev_notdef(),			/* 24: */
cdev_wsdisplay_init(NWSDISPLAY, wsdisplay),	/* 25: */
	cdev_mouse_init(NWSKBD, wskbd),	/* 26: */
	cdev_mouse_init(NWSMOUSE, wsmouse),	/* 27: */
	cdev_mouse_init(NWSMUX, wsmux),	/* 28: */
#ifdef USER_PCICONF
	cdev_pci_init(NPCI,pci),	/* 29: PCI user */
#else
	cdev_notdef(),			/* 29 */
#endif
	cdev_notdef(),			/* 30: */
	cdev_pf_init(NPF,pf),		/* 31: packet filter */
	cdev_uk_init(NUK,uk),		/* 32: unknown SCSI */
	cdev_random_init(1,random),	/* 33: random data source */
	cdev_ss_init(NSS,ss),		/* 34: SCSI scanner */
	cdev_ksyms_init(NKSYMS,ksyms),	/* 35: Kernel symbols device */
	cdev_ch_init(NCH,ch),		/* 36: SCSI autochanger */
	cdev_notdef(),			/* 37: */
	cdev_notdef(),			/* 38: */
	cdev_notdef(),			/* 39: */
	cdev_notdef(),			/* 40: */
	cdev_notdef(),			/* 41: */
	cdev_notdef(),			/* 42: */
	cdev_notdef(),			/* 33: */
	cdev_audio_init(NAUDIO,audio),	/* 44: /dev/audio */
	cdev_video_init(NVIDEO,video),	/* 45: generic video I/O */
	cdev_notdef(),			/* 46: */
	cdev_crypto_init(NCRYPTO,crypto),	/* 47: /dev/crypto */
	cdev_notdef(),			/* 48: */
	cdev_bio_init(NBIO,bio),	/* 49: ioctl tunnel */
	cdev_systrace_init(NSYSTRACE,systrace),	/* 50: system call tracing */
#ifdef XFS
	cdev_xfs_init(NXFS,xfs_dev),	/* 51: xfs communication device */
#else
	cdev_notdef(),			/* 51: */
#endif
	cdev_ptm_init(NPTY,ptm),	/* 52: pseudo-tty ptm device */
};

int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(1, 0);

/*
 * Routine that identifies /dev/mem and /dev/kmem.
 *
 * A minimal stub routine can always return 0.
 */
int
iskmemdev(dev)
	dev_t dev;
{

	if (major(dev) == 3 && (minor(dev) == 0 || minor(dev) == 1))
		return (1);
	return (0);
}

/*
 * Returns true if def is /dev/zero
 */
int
iszerodev(dev)
	dev_t dev;
{
	return (major(dev) == 3 && minor(dev) == 12);
}

dev_t
getnulldev()
{
	return(makedev(3, 2));
}


int chrtoblktbl[] =  {
	/* VCHR		VBLK */
	/* 0 */		NODEV,
	/* 1 */		NODEV,
	/* 2 */		NODEV,
	/* 3 */		NODEV,
	/* 4 */		NODEV,
	/* 5 */		NODEV,
	/* 6 */		NODEV,
	/* 7 */		NODEV,
	/* 8 */		3,		/* cd */
	/* 9 */		0,		/* sd */
	/* 10 */	10,		/* st */
	/* 11 */	2,		/* vnd */
	/* 12 */	NODEV,
	/* 13 */	NODEV,
	/* 14 */	NODEV,
	/* 15 */	NODEV,
	/* 16 */	NODEV,
	/* 17 */	NODEV,
	/* 18 */	4,		/* wd */
	/* 19 */	NODEV,
	/* 20 */	NODEV,
	/* 21 */	NODEV,
	/* 22 */	8,		/* rd */
};

int nchrtoblktbl = sizeof(chrtoblktbl) / sizeof(int);

/*
 * This entire table could be autoconfig()ed but that would mean that
 * the kernel's idea of the console would be out of sync with that of
 * the standalone boot.  I think it best that they both use the same
 * known algorithm unless we see a pressing need otherwise.
 */
#include <dev/cons.h>

cons_decl(ws);
cons_decl(com);

struct	consdev constab[] = {
#if NWSDISPLAY > 0
	cons_init(ws),
#endif
#if NCOM > 0
	cons_init(com),
#endif
	{ 0 },
};
