/*
 * Copyright (c) 1994-1998 Mark Brinicombe.
 * All rights reserved.
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
 *	This product includes software developed by Mark Brinicombe
 *	for the NetBSD Project.
 * 4. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * RiscBSD kernel project
 *
 * conf.c
 *
 * Character and Block Device configuration
 * Console configuration
 *
 * Defines the structures cdevsw and constab
 *
 * Created : 17/09/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vnode.h>

#include <machine/conf.h>

#include "inet.h"

/*
 * From this point, these need to be MI foo.h files.
 */

/*
 * Standard MI devices (e.g. ones in dev/ic)
 */
#include "com.h"		/* NS164x0 serial ports */

/*
 * Standard pseudo-devices
 */
#include "bpfilter.h"
#include "pf.h"
#include "pty.h"
#include "tun.h"
#include "ksyms.h"

/*
 * APM interface
 */
#ifdef CONF_HAVE_APM
#include "apm.h"
#else
#define NAPM	0
#endif

/*
 * Disk/Filesystem pseudo-devices
 */
#include "rd.h"				/* memory disk driver */
#include "vnd.h"			/* vnode disk driver */

/*
 * WD/ATA devices
 */
#include "wd.h"
bdev_decl(wd);
bdev_decl(sw);

#ifdef USER_PCICONF
#include "pci.h"
cdev_decl(pci);
#endif

/*
 * SCSI/ATAPI devices
 */
#include "sd.h"
#include "st.h"
#include "cd.h"
#include "ch.h"
#include "uk.h"
#include "ss.h"
#include "bio.h"

/*
 * Audio devices
 */
#include "audio.h"
#include "video.h"
#include "midi.h"
#include "sequencer.h"

/*
 * USB devices
 */
#include "usb.h"
#include "ucom.h"
#include "ugen.h"
#include "uhid.h"
#include "ulpt.h"
#include "urio.h"
#include "uscanner.h"

/*
 * WSCONS devices
 */
#include "wsdisplay.h"
/*
#include "wsfont.h"
*/
#include "wskbd.h"
#include "wsmouse.h"
#include "wsmux.h"
cdev_decl(wskbd);
cdev_decl(wsmouse);

#include "lpt.h"
#ifdef CONF_HAVE_FCOM
#include "fcom.h"
#else
#define NFCOM	0
#endif

#include "radio.h"
cdev_decl(radio);

#include <arm/conf.h>

/* Block devices */

struct bdevsw bdevsw[] = {
	bdev_lkm_dummy(),		/*  0: */
	bdev_swap_init(1, sw),		/*  1: swap pseudo-device */
	bdev_lkm_dummy(),		/*  2: */
	bdev_lkm_dummy(),		/*  3: */
	bdev_lkm_dummy(),		/*  4: */
	bdev_lkm_dummy(),		/*  5: */
	bdev_lkm_dummy(),		/*  6: */
	bdev_lkm_dummy(),		/*  7: */
	bdev_lkm_dummy(),		/*  8: */
	bdev_lkm_dummy(),		/*  9: */
	bdev_lkm_dummy(),		/* 10: */
	bdev_lkm_dummy(),		/* 11: */
	bdev_lkm_dummy(),		/* 12: */
	bdev_lkm_dummy(),		/* 13: */
	bdev_lkm_dummy(),		/* 14: */
	bdev_lkm_dummy(),		/* 15: */
	bdev_disk_init(NWD,wd),		/* 16: Internal IDE disk */
	bdev_lkm_dummy(),		/* 17: */
	bdev_disk_init(NRD,rd),		/* 18: memory disk */
	bdev_disk_init(NVND,vnd),	/* 19: vnode disk driver */
	bdev_lkm_dummy(),		/* 20: */
	bdev_lkm_dummy(),		/* 21: */
	bdev_lkm_dummy(),		/* 22: */
	bdev_lkm_dummy(),		/* 23: */
	bdev_disk_init(NSD,sd),		/* 24: SCSI disk */
	bdev_tape_init(NST,st),		/* 25: SCSI tape */
	bdev_disk_init(NCD,cd),		/* 26: SCSI cdrom */
	bdev_lkm_dummy(),		/* 27: */
	bdev_lkm_dummy(),		/* 28: */
	bdev_lkm_dummy(),		/* 29: */
	bdev_lkm_dummy(),		/* 30: */
	bdev_lkm_dummy(),		/* 31: */
	bdev_lkm_dummy(),		/* 32: */
};

/* Character devices */
#define ptstty		ptytty
#define ptsioctl	ptyioctl
#define ptctty		ptytty
#define ptcioctl	ptyioctl

#ifdef XFS
#include <xfs/nxfs.h>
cdev_decl(xfs_dev);
#endif
#include "systrace.h"

#include "hotplug.h"

#ifdef CONF_HAVE_GPIO
#include "gpio.h"
#else
#define	NGPIO 0
#endif

#ifdef CONF_HAVE_SPKR
#include "spkr.h"
#else
#define	NSPKR 0
#endif

struct cdevsw cdevsw[] = {
	cdev_cn_init(1,cn),			/*  0: virtual console */
	cdev_ctty_init(1,ctty),			/*  1: controlling terminal */
	cdev_mm_init(1,mm),			/*  2: /dev/{null,mem,kmem,...} */
	cdev_swap_init(1,sw),			/*  3: /dev/drum (swap pseudo-device) */
	cdev_tty_init(NPTY,pts),		/*  4: pseudo-tty slave */
	cdev_ptc_init(NPTY,ptc),		/*  5: pseudo-tty master */
	cdev_log_init(1,log),			/*  6: /dev/klog */
	cdev_fd_init(1,filedesc),		/*  7: file descriptor pseudo-device */
	cdev_ksyms_init(NKSYMS,ksyms),		/*  8: Kernel symbols device */
	cdev_lpt_init(NLPT,lpt),		/*  9: parallel printer */
	cdev_lkm_dummy(),			/* 10: */
	cdev_lkm_dummy(),			/* 11: */
	cdev_tty_init(NCOM,com),		/* 12: serial port */
	cdev_gpio_init(NGPIO,gpio),		/* 13: GPIO interface */
	cdev_lkm_dummy(),			/* 14: */
	cdev_lkm_dummy(),			/* 15: */
	cdev_disk_init(NWD,wd),			/* 16: ST506/ESDI/IDE disk */
	cdev_lkm_dummy(),			/* 17: */
	cdev_disk_init(NRD,rd),			/* 18: ram disk driver */
	cdev_disk_init(NVND,vnd),		/* 19: vnode disk driver */
	cdev_lkm_dummy(),			/* 20: */
	cdev_lkm_dummy(),			/* 21: */
	cdev_bpf_init(NBPFILTER,bpf),		/* 22: Berkeley packet filter */
	cdev_lkm_dummy(),			/* 23: */
	cdev_disk_init(NSD,sd),			/* 24: SCSI disk */
	cdev_tape_init(NST,st),			/* 25: SCSI tape */
	cdev_disk_init(NCD,cd),			/* 26: SCSI CD-ROM */
	cdev_ch_init(NCH,ch),			/* 27: SCSI autochanger */
	cdev_uk_init(NUK,uk),			/* 28: SCSI unknown */
	cdev_scanner_init(NSS,ss),		/* 29: SCSI scanner */
	cdev_lkm_dummy(),			/* 30: */
	cdev_lkm_dummy(),			/* 31: */
	cdev_lkm_dummy(),			/* 32: */
	cdev_tun_init(NTUN,tun),		/* 33: network tunnel */
	cdev_apm_init(NAPM,apm),		/* 34: APM interface */
	cdev_lkm_init(NLKM,lkm),		/* 35: loadable module driver */
	cdev_audio_init(NAUDIO,audio),		/* 36: generic audio I/O */
	cdev_hotplug_init(NHOTPLUG,hotplug),	/* 37: devices hot plugging*/
	cdev_video_init(NVIDEO,video),		/* 38: generic video I/O */
	cdev_lkm_dummy(),			/* 39: reserved */
	cdev_random_init(1,random),		/* 40: random generator */
	cdev_lkm_dummy(),			/* 41: reserved */
	cdev_lkm_dummy(),			/* 42: reserved */
	cdev_lkm_dummy(),			/* 43: reserved */
	cdev_lkm_dummy(),			/* 44: reserved */
	cdev_lkm_dummy(),			/* 45: reserved */
	cdev_pf_init(NPF,pf),			/* 46: packet filter */
	cdev_crypto_init(NCRYPTO,crypto),	/* 47: /dev/crypto */
	cdev_lkm_dummy(),			/* 48: reserved */
	cdev_lkm_dummy(),			/* 49: reserved */
	cdev_systrace_init(NSYSTRACE,systrace),	/* 50: system call tracing */
#ifdef XFS
	cdev_xfs_init(NXFS,xfs_dev),		/* 51: xfs communication device */
#else
	cdev_notdef(),				/* 51: reserved */
#endif
	cdev_bio_init(NBIO,bio),		/* 52: ioctl tunnel */
	cdev_notdef(),				/* 53: reserved */
	cdev_tty_init(NFCOM,fcom),		/* 54: FOOTBRIDGE console */
	cdev_lkm_dummy(),			/* 55: Reserved for bypass device */
	cdev_notdef(),				/* 56: reserved */
	cdev_midi_init(NMIDI,midi),		/* 57: MIDI I/O */
	cdev_midi_init(NSEQUENCER,sequencer),	/* 58: sequencer I/O */
	cdev_notdef(),				/* 59: reserved */
	cdev_wsdisplay_init(NWSDISPLAY,wsdisplay), /* 60: frame buffers, etc.*/
	cdev_mouse_init(NWSKBD,wskbd),		/* 61: keyboards */
	cdev_mouse_init(NWSMOUSE,wsmouse),	/* 62: mice */
	cdev_mouse_init(NWSMUX,wsmux),		/* 63: ws multiplexor */
	cdev_usb_init(NUSB,usb),		/* 64: USB controller */
	cdev_usbdev_init(NUHID,uhid),		/* 65: USB generic HID */
	cdev_lpt_init(NULPT,ulpt),		/* 66: USB printer */
	cdev_urio_init(NURIO,urio),		/* 67: Diamond Rio 500 */
	cdev_tty_init(NUCOM,ucom),		/* 68: USB tty */
	cdev_usbdev_init(NUSCANNER,uscanner),	/* 69: USB scanner */
	cdev_usbdev_init(NUGEN,ugen),		/* 70: USB generic driver */
	cdev_lkm_dummy(),			/* 71: reserved */
	cdev_lkm_dummy(),			/* 72: reserved */
	cdev_lkm_dummy(),			/* 73: reserved */
	cdev_lkm_dummy(),			/* 74: reserved */
	cdev_lkm_dummy(),			/* 75: reserved */
	cdev_lkm_dummy(),			/* 76: reserved */
	cdev_notdef(),				/* 77: removed device */
	cdev_notdef(),				/* 78: removed device */
	cdev_notdef(),				/* 79: removed device */
	cdev_notdef(),				/* 80: removed device */
	cdev_notdef(),				/* 81: removed device */
	cdev_notdef(),				/* 82: removed device */
	cdev_notdef(),				/* 83: removed device */
	cdev_notdef(),				/* 84: removed device */
	cdev_notdef(),				/* 85: removed device */
	cdev_notdef(),				/* 86: removed device */
	cdev_notdef(),				/* 87: removed device */
#ifdef USER_PCICONF
	cdev_pci_init(NPCI,pci),		/* 88: PCI user */
#else
	cdev_notdef(),
#endif
	cdev_notdef(),				/* 89: removed device */
	cdev_notdef(),				/* 90: removed device */
	cdev_notdef(),				/* 91: removed device */
	cdev_notdef(),				/* 92: removed device */
	cdev_notdef(),				/* 93: removed device */
	cdev_notdef(),				/* 94: removed device */
	cdev_notdef(),				/* 95: removed device */
	cdev_notdef(),				/* 96: removed device */
	cdev_radio_init(NRADIO,radio),		/* 97: generic radio I/O */
	cdev_ptm_init(NPTY,ptm),		/* 98: pseudo-tty ptm device */
	cdev_spkr_init(NSPKR,spkr),		/* 99: PC speaker */
};

int nblkdev = sizeof(bdevsw) / sizeof(bdevsw[0]);
int nchrdev = sizeof(cdevsw) / sizeof(cdevsw[0]);

int mem_no = 2;		/* major device number of memory special file */

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
 * Returns true if dev is /dev/mem or /dev/kmem.
 */
int
iskmemdev(dev)
	dev_t dev;
{
	return (major(dev) == mem_no && minor(dev) < 2);
}

/*
 * Returns true if dev is /dev/zero.
 */
int
iszerodev(dev)
	dev_t dev;
{
	return (major(dev) == mem_no && minor(dev) == 3);
}


int chrtoblktbl[] = {
/* XXXX This needs to be dynamic for LKMs. */
    /*VCHR*/	/*VBLK*/
    /*  0 */	NODEV,
    /*  1 */	NODEV,
    /*  2 */	NODEV,
    /*  3 */	NODEV,
    /*  4 */	NODEV,
    /*  5 */	NODEV,
    /*  6 */	NODEV,
    /*  7 */	NODEV,
    /*  8 */	NODEV,
    /*  9 */	NODEV,
    /* 10 */	NODEV,
    /* 11 */	NODEV,
    /* 12 */	NODEV,
    /* 13 */	NODEV,
    /* 14 */	NODEV,
    /* 15 */	NODEV,
    /* 16 */	16,		/* wd */
    /* 17 */	NODEV,
    /* 18 */	18,		/* rd */
    /* 19 */	19,		/* vnd */
    /* 20 */	NODEV,
    /* 21 */	NODEV,
    /* 22 */	NODEV,
    /* 23 */	NODEV,
    /* 24 */	24,		/* sd */
    /* 25 */	25,		/* st */
    /* 26 */	26,		/* cd */
};
int nchrtoblktbl = sizeof(chrtoblktbl) / sizeof(chrtoblktbl[0]);

dev_t
getnulldev()
{
	return makedev(mem_no, 2);
}
