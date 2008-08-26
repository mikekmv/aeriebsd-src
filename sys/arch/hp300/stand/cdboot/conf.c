
/*
 * Copyright (c) 1982, 1986, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 *
 *	@(#)conf.c	8.1 (Berkeley) 6/10/93
 */

#include <sys/param.h>

#include <lib/libsa/stand.h>

#include "samachdep.h"

#include <lib/libsa/cd9660.h>

int	debug = 0;	/* XXX */

/*
 * Device configuration
 */
int	sdstrategy(void *, int, daddr_t, size_t, void *, size_t *);
int	sdopen(struct open_file *, ...);
int	sdclose(struct open_file *);
#define	sdioctl		noioctl

#define xxstrategy	\
	(int (*)(void *, int, daddr_t, size_t, void *, size_t *))nullsys
#define xxopen		(int (*)(struct open_file *, ...))nodev
#define xxclose		(int (*)(struct open_file *))nullsys

struct devsw devsw[] = {
	{ "??",	xxstrategy,	xxopen,	xxclose,	noioctl }, /*0*/
	{ "??",	xxstrategy,	xxopen,	xxclose,	noioctl }, /*1*/
	{ "??",	xxstrategy,	xxopen,	xxclose,	noioctl }, /*2*/
	{ "??",	xxstrategy,	xxopen,	xxclose,	noioctl }, /*3*/
	{ "sd",	sdstrategy,	sdopen,	sdclose,	sdioctl }, /*4*/
};
int	ndevs = (sizeof(devsw) / sizeof(devsw[0]));

/*
 * Physical unit/lun detection.
 */
int	punitzero(int, int, int *);

int
punitzero(int ctlr, int slave, int *punit)
{

	*punit = 0;
	return (0);
}

#define	xxpunit		punitzero
#define	sdpunit		punitzero

struct punitsw punitsw[] = {
	{ xxpunit },
	{ xxpunit },
	{ xxpunit },
	{ xxpunit },
	{ sdpunit },
};
int	npunit = (sizeof(punitsw) / sizeof(punitsw[0]));

/*
 * Filesystem configuration
 */
struct fs_ops file_system_cd9660[] = {
	{ cd9660_open, cd9660_close, cd9660_read, cd9660_write, cd9660_seek,
	  cd9660_stat },
};

struct fs_ops file_system[2];
int	nfsys = 1;		/* default; changed per device type. */
