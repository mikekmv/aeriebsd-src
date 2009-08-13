/*
 * Copyright (c) 2009 Michael Shalayeff
 * Copyright (c) 2009 Pedro Martelletto
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
#include <sys/disklabel.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/tree.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util.h>

#include <milfs/milfs.h>

#define	plural(n)	((n) == 1 ? "" : "s")

char *rpath;
int bset, cset, fd;
u_int32_t bsize = MILFS_BSIZE_DEFAULT;
u_int32_t cgsize = MILFS_CGSIZE_DEFAULT;
u_int32_t ncg, blkpercg;

__dead void
usage(void)
{
	extern char *__progname;
	fprintf(stderr, "usage: %s [-b block-size] [-c cylinder-group-size] "
	    "disk\n", __progname);
	exit(1);
}

/*
 * See if 'n' is a power of 2. If it is, return log(n) in base 2. Otherwise,
 * return -1.
 */
int
log2(int n)
{
	int i, got1, nbits;

	got1 = 0;
	nbits = sizeof(n) * NBBY;

	for (i = 0; i < nbits; i++)
		if (n & 1 << i) {
			if (got1)
				return (-1);
			got1 = i + 1;
		}

	return (got1 - 1);
}

/*
 * Check the disklabel. The partition must be a MILFS partition. Additionally,
 * the disk parameters cannot conflict with the file system's.
 */
void
cklabel(void)
{
	struct disklabel dklabel;
	int i;

	if (ioctl(fd, DIOCGDINFO, &dklabel) < 0)
		err(1, "%s: ioctl", rpath);

	/* Get partition index. */
	i = rpath[strlen(rpath) - 1] - 'a';

	if (i >= dklabel.d_npartitions)
		errx(1, "%s: bad label", rpath);

	if (dklabel.d_partitions[i].p_fstype != FS_MILFS)
		errx(1, "%s: not a MILFS partition", rpath);

	if (dklabel.d_secsize > bsize)
		errx(1, "%s: file system block size can't be less than the "
		    "device sector size", rpath);

	if (bsize % dklabel.d_secsize)
		errx(1, "%s: file system block size must be a multiple of the "
		    "device sector size", rpath);

	ncg = ((DL_GETPSIZE(&dklabel.d_partitions[i]) * dklabel.d_secsize) -
	    MILFS_BBSIZE) / cgsize;

	if (ncg == 0)
		errx(1, "%s: cylinder group size is too large for partition",
		    rpath);

	blkpercg = cgsize / bsize;
}

/*
 * See if the file system parameters match the requirements of MILFS. Note
 * that preliminary checks are performed in main() just after getopt().
 */
void
ckparam(void)
{
	int i, j;

	if (bsize != MILFS_BSIZE_DEFAULT) {
		j = log2(bsize);
		if (j == -1)
			errx(1, "invalid block size");
		if (cset == 0)
			cgsize = 1 << (2*j - 5);
	}

	if (cgsize != MILFS_CGSIZE_DEFAULT) {
		i = log2(cgsize);
		if (i == -1 || (i & 1) == 0)
			errx(1, "invalid cylinder group size");
		if (bset == 0)
			bsize = 1 << ((5 + i) / 2);
	}

	if (log2(bsize) != (5 + log2(cgsize)) / 2)
		errx(1, "incompatible block and cylinder group sizes");
}

/*
 * Clear the cylinder group space, so that the kernel scanning code doesn't
 * get confused by previous MILFS file systems on the same partition. This
 * requires iterating over cylinder group sizes smaller than the desired and
 * erasing their descriptors.
 */
void
clrcg(char *buf)
{
	int i, j, max, min;
	off_t pos;
	u_int32_t xcgsize;

	min = log2(MILFS_CGSIZE_MIN);
	max = log2(MILFS_CGSIZE_MAX);

	j = log2(cgsize);
	if (j < min || j > max)
		errx(1, "invalid cylinder group size");

	memset(buf, 0, bsize);

	for (i = min; i < j; i += 2) {
		/*
		 * If the size of the cylinder group indexed by 'i' is less
		 * than or equal to the current block size, then the first
		 * write will already have it clobbered.
		 */
		xcgsize = 1 << i;
		if (xcgsize <= bsize)
			continue;
		pos = MILFS_BBSIZE + xcgsize - bsize;
		if (lseek(fd, pos, SEEK_SET) != pos)
			err(1, "lseek2");
		if (write(fd, buf, bsize) != bsize)
			err(1, "write2");
	}
}

/*
 * Allocate the root inode. This is done in two steps; see below.
 */
void
mkroot(void)
{
	char *buf;
	off_t pos;
	struct milfs_sinode *sip;
	struct milfs_dinode *dip;
	struct milfs_cgdesc *cdp;
	struct timeval tv;

	/*
	 * Leave space for the boot blocks.
	 */
	if (lseek(fd, MILFS_BBSIZE, SEEK_SET) != MILFS_BBSIZE)
		err(1, "lseek1");

	buf = malloc(bsize);
	if (buf == NULL)
		err(1, "malloc1");

	bzero(buf, bsize);

	if (gettimeofday(&tv, NULL) == -1)
		err(1, "gettimeofday");

	sip = (struct milfs_sinode *)buf;
	sip->si_inode = MILFS_INODE_ROOT;
	sip->si_birthsec = tv.tv_sec;
	sip->si_birthnsec = tv.tv_usec * 1000;
	sip->si_changesec = tv.tv_sec;
	sip->si_changeusec = tv.tv_usec;
	sip->si_gen = 0;
	sip->si_xuid = geteuid();
	sip->si_xgid = getegid();
	sip->si_mode = MILFS_MODE_DIRECTORY;
	sip->si_nlink = 2;

	/*
	 * Step 1: Write a static inode entry for the root inode at the first
	 * block of the first cylinder group.
	 */
	if (write(fd, sip, bsize) != bsize)
		err(1, "%s: write1", rpath);

	clrcg(buf);

	/*
	 * Seek to the last block of the first cylinder group. This is where
	 * the dynamic inode entries and the cylinder group descriptor are
	 * stored.
	 */
	pos = MILFS_BBSIZE + cgsize - bsize;
	if (lseek(fd, pos, SEEK_SET) != pos)
		err(1, "%s: lseek3", rpath);

	dip = (struct milfs_dinode *)buf;
	dip->di_inode = MILFS_INODE_DIRECTORY;
	dip->di_offset = 0;
	dip->di_modsec = tv.tv_sec;
	dip->di_modusec = tv.tv_usec;
	dip->di_gen = 0;
	dip->di_size = sizeof(struct milfs_sinode);

	cdp = (struct milfs_cgdesc *)((char *)dip + bsize -
	    sizeof(struct milfs_dinode));

	cdp->cd_bsize = bsize;
	cdp->cd_cgsize = cgsize;
	cdp->cd_magic = MILFS_CGDESC_MAGIC;

	/*
	 * Step 2: Write a dynamic inode entry for the static inode and the
	 * cylinder group descriptor.
	 */
	if (write(fd, dip, bsize) != bsize)
		err(1, "%s: write3", rpath);

	free(buf);
}

int
main(int argc, char **argv)
{
	const char *errstr;
	char buf[FMT_SCALED_STRSIZE];
	int ch;
	struct stat st;

	bset = cset = 0;

	while ((ch = getopt(argc, argv, "b:c:ht:")) != -1) {
		switch (ch) {
		case 'b':
			bsize = strtonum(optarg, MILFS_BSIZE_MIN,
			    MILFS_BSIZE_MAX, &errstr);
			if (errstr)
				errx(1, "value for option -b is %s", errstr);
			bset = 1;
			break;
		case 'c':
			cgsize = strtonum(optarg, MILFS_CGSIZE_MIN,
			    MILFS_CGSIZE_MAX, &errstr);
			if (errstr)
				errx(1, "value for option -c is %s", errstr);
			cset = 1;
			break;
		case 'h':
			usage();
		case 't':
			/* Compatibility with 'newfs -t milfs'. */
			if (strcmp(optarg, "milfs"))
				errx(1, "invalid argument for option -t");
			break;
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	fd = opendev(argv[0], O_WRONLY, OPENDEV_PART, &rpath);
	if (fd < 0)
		err(1, "opendev");

	if (fstat(fd, &st) < 0)
		err(1, "fstat");

	if (S_ISBLK(st.st_mode))
		errx(1, "%s: raw device required", rpath);

	ckparam();	/* Check the file system parameters. */
	cklabel();	/* Check the disklabel. */
	mkroot();	/* Allocate the root inode. */

	if (close(fd) == -1)
		err(1, "%s: close", rpath);

	if (fmt_scaled((long long)blkpercg * ncg * bsize, buf)) {
		/* fmt_scaled() failed */
		fprintf(stdout, "%s: %lldB in %d blocks of %d bytes\n", rpath,
		    (long long)blkpercg * ncg * bsize, blkpercg * ncg, bsize);
	} else {
		fprintf(stdout, "%s: %s in %d blocks of %d bytes\n", rpath, buf,
		    blkpercg * ncg, bsize);
	}

	fprintf(stdout, "%s: %d cylinder group%s of %d block%s\n", rpath, ncg,
	    plural(ncg), blkpercg, plural(blkpercg));

	exit(0);
}
