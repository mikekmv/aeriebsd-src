/*
 * Copyright (c) 2006 Michael Shalayeff
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

#ifndef lint
static const char rcsid[] = "$ABSD: sysctl.c,v 1.2 2010/06/05 11:33:16 mickey Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/namei.h>
#include <sys/vmmeter.h>
#include <sys/tty.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mount.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/ip_ipip.h>
#include <netinet/ip_gre.h>
#include <netinet/ip_esp.h>
#include <netinet/ip_ah.h>
#include <netinet/ip_ipcomp.h>
#include <netinet/ip_ether.h>
#include <netinet/ip_carp.h>
#include <netinet/icmp6.h>
#include <netinet6/pim6_var.h>

#include <ddb/db_var.h>

#include <uvm/uvm_swap_encrypt.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/ffs_extern.h>

#include <nfs/rpcv2.h>
#include <nfs/nfsproto.h>
#include <nfs/nfs.h>

#include <string.h>
#include <stdio.h>

#include <machine/cpu.h>
#ifndef	CTL_MACHDEP_NAMES
#define	CTL_MACHDEP_NAMES	{ 0 }
#endif
#ifndef	CPU_MAXID
#define	CPU_MAXID		0
#endif

#define	EMUL_MAXID		16

const struct ctlname root[] = CTL_NAMES;
const struct ctlname kern[] = CTL_KERN_NAMES;
const struct ctlname kmalloc[] = CTL_KERN_MALLOC_NAMES;
      char *kmemnames[] = INITKMEMNAMES;
      struct ctlname kmems[M_LAST];
const struct ctlname nchstats[] = CTL_KERN_NCHSTATS_NAMES;
const struct ctlname forkstat[] = CTL_KERN_FORKSTAT_NAMES;
const struct ctlname tty[] = CTL_KERN_TTY_NAMES;
const struct ctlname seminfo[] = CTL_KERN_SEMINFO_NAMES;
const struct ctlname shminfo[] = CTL_KERN_SHMINFO_NAMES;
const struct ctlname intrcnt[] = CTL_KERN_INTRCNT_NAMES;
const struct ctlname wdog[] = CTL_KERN_WATCHDOG_NAMES;
const struct ctlname tcount[] = CTL_KERN_TIMECOUNTER_NAMES;
      struct ctlname emul[EMUL_MAXID] = {{ "nemuls",	0 }};
const struct ctlname emulnames[] = CTL_KERN_EMUL_NAMES;
const struct ctlname procargs[] = CTL_KERN_PROCARGS_NAMES;
const struct ctlname vm[] = CTL_VM_NAMES;
const struct ctlname vmswen[] = CTL_SWPENC_NAMES;
const struct ctlname fs[] = CTL_FS_NAMES;
const struct ctlname posix[] = CTL_FS_POSIX_NAMES;
const struct ctlname net[] = CTL_NET_NAMES;
const struct ctlname ipproto[] = CTL_IPPROTO_NAMES;
const struct ctlname ip[] = IPCTL_NAMES;
const struct ctlname ifq[] = CTL_IFQ_NAMES;
const struct ctlname icmp[] = ICMPCTL_NAMES;
const struct ctlname ipip[] = IPIPCTL_NAMES;
const struct ctlname tcp[] = TCPCTL_NAMES;
const struct ctlname udp[] = UDPCTL_NAMES;
const struct ctlname gre[] = GRECTL_NAMES;
const struct ctlname esp[] = ESPCTL_NAMES;
const struct ctlname ah[] = AHCTL_NAMES;
const struct ctlname mip[] = MOBILEIPCTL_NAMES;
const struct ctlname eip[] = ETHERIPCTL_NAMES;
const struct ctlname ipcomp[] = IPCOMPCTL_NAMES;
const struct ctlname carp[] = CARPCTL_NAMES;
const struct ctlname route[] = CTL_NET_RT_NAMES;
const struct ctlname inet6[] = IPV6CTL_NAMES;
const struct ctlname ip6proto[] = CTL_IPV6PROTO_NAMES;
const struct ctlname ip6[] = IPV6CTL_NAMES;
const struct ctlname icmp6[] = ICMPV6CTL_NAMES;
const struct ctlname pim6[] = PIM6CTL_NAMES;
const struct ctlname pfkey[] = CTL_NET_KEY_NAMES;
const struct ctlname bpf[] = CTL_NET_BPF_NAMES;
const struct ctlname debug[] =  { 0 };
const struct ctlname hw[] = CTL_HW_NAMES;
const struct ctlname md[] = CTL_MACHDEP_NAMES;
const struct ctlname user[] = CTL_USER_NAMES;
const struct ctlname ddb[] = CTL_DDB_NAMES;
const struct ctlname vfs[] = {
	{ "gen",	0 },	/* fake list to just produce the names */
	{ "ffs",	0 },
	{ "nfs",	0 },
	{ "mfs",	0 },
	{ "msdos",	0 },
	{ "lfs",	0 },
	{ "ntfs",	0 },
	{ "fdesc",	0 },
	{ "portal",	0 },
	{ "null",	0 },
	{ "umap",	0 },
	{ "kern",	0 },
	{ "proc",	0 },
	{ "udf",	0 },
	{ "cd9660",	0 },
	{ "union",	0 },
	{ "ados",	0 },
	{ "ext2",	0 },
	{ NULL },	/* 18 */
	{ NULL },	/* 19 */
	{ NULL },	/* 20 */
	{ "xfs",	0 },
};
const struct ctlname genfs[] = CTL_VFSGENCTL_NAMES;
const struct ctlname ffsname[] = FFS_NAMES;
const struct ctlname nfsname[] = FS_NFS_NAMES;
const struct ctlname empty[] = {{ NULL }};

struct ctlist {
	const struct ctlname *names;
	const struct ctlist *lists;
	int maxid;
};

const struct ctlist kmemlists[] = {
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ kmems,	NULL,	M_LAST }
}, emulists[EMUL_MAXID] = {
	{ NULL },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
	{ emulnames,	NULL,	KERN_EMUL_MAXID },
}, pargslist[] = {
	{ procargs,	NULL,	KERN_PROCARGS_MAXID }
}, kernlists[KERN_MAXID] = {
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },	/* proc */
	{ NULL },
	{ NULL },	/* prof */
	{ NULL },	/* posix */
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ kmalloc,	kmemlists,	KERN_MALLOC_MAXID },
	{ NULL },
	{ nchstats,	NULL,	KERN_NCHSTATS_MAXID },
	{ forkstat,	NULL,	KERN_FORKSTAT_MAXID },
	{ NULL },
	{ tty,		NULL,	KERN_TTY_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },	/* pool */
	{ NULL },
	{ NULL },	/* ipc */
	{ NULL },
	{ NULL },
	{ NULL },
	{ empty,	pargslist,	-1 },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ seminfo,	NULL,	KERN_SEMINFO_MAXID },
	{ shminfo,	NULL,	KERN_SHMINFO_MAXID },
	{ intrcnt,	NULL,	KERN_INTRCNT_MAXID  },
	{ wdog,		NULL,	KERN_WATCHDOG_MAXID },
	{ emul,		emulists,	EMUL_MAXID },
	{ NULL },	/* proc2 */
	{ NULL },
	{ NULL },	/* evcount */
	{ tcount,	NULL,	KERN_TIMECOUNTER_MAXID },
}, vmlists[VM_MAXID] = {
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ vmswen,	NULL,	SWPENC_MAXID },
}, fslists[FS_MAXID] = {
	{ NULL },
	{ posix,	NULL,	FS_POSIX_MAXID },
}, iplists[IPCTL_MAXID] = {
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ ifq,		NULL,	IFQCTL_MAXID },
	{ NULL },
	{ NULL },
}, inetlists[IPPROTO_MAXID] = {
	{ ip,		iplists,IPPROTO_MAXID },
	{ icmp,		NULL,	ICMPCTL_MAXID },
	{ NULL },	/* igmp */
	{ NULL },	/* ggp */
	{ ipip,		NULL,	IPIPCTL_MAXID },
	{ NULL },
	{ tcp,		NULL,	TCPCTL_MAXID },
	{ NULL },
	{ NULL },	/* egp */
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },	/* pup */
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ udp,		NULL,	UDPCTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ gre,		NULL,	GRECTL_MAXID },
	{ NULL },
	{ NULL },
	{ esp,		NULL,	ESPCTL_MAXID },
	{ ah,		NULL,	AHCTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ mip,		NULL,	MOBILEIPCTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ eip,		NULL,	ETHERIPCTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ ipcomp,	NULL,	IPCOMPCTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ carp,		NULL,	CARPCTL_MAXID },
}, inet6lists[IPV6PROTO_MAXID] = {
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },	/* tcp6,		NULL,	IPV6PROTO_MAXID }, */
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },	/* udp6,		NULL,	IPV6PROTO_MAXID }, */
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ ip6,		NULL,	IPV6CTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },	/* ipsec6,	NULL,	IPV6CTL_MAXID }, */
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ icmp6,	NULL,	ICMPV6CTL_MAXID },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ pim6,		NULL,	PIM6CTL_MAXID },
}, netlists[NET_MAXID] = {
	{ NULL },
	{ NULL },	/* unix */
	{ ipproto,	inetlists,	IPPROTO_MAXID },
	{ NULL },	/* implink */
	{ NULL },	/* pup */
	{ NULL },	/* chaos */
	{ NULL },	/* xns */
	{ NULL },	/* iso */
	{ NULL },	/* ymca */
	{ NULL },	/* datakit */
	{ NULL },	/* ccitt */
	{ NULL },	/* sna */
	{ NULL },	/* decnet */
	{ NULL },	/* dli */
	{ NULL },	/* lat */
	{ NULL },	/* hylink */
	{ NULL },	/* appletalk */
	{ route,	NULL,	NET_RT_MAXID },
	{ NULL },	/* link */
	{ NULL },	/* xtp */
	{ NULL },	/* coip */
	{ NULL },	/* cnt */
	{ NULL },	/* rtip */
	{ NULL },	/* ipx */
	{ ip6proto,	inet6lists,	IPV6PROTO_MAXID },
	{ NULL },	/* pip */
	{ NULL },	/* isdn */
	{ NULL },	/* natm */
	{ NULL },	/* encap */
	{ NULL },	/* sip */
	{ pfkey,	NULL,	NET_KEY_MAXID },
	{ bpf,		NULL,	NET_BPF_MAXID }
}, vfslists[] = {
	/* this order comes fomr vfsconf.vfc_typenum */
	{ genfs,	NULL,	VFS_MAXID },
	{ ffsname,	NULL,	FFS_MAXID },
	{ nfsname,	NULL,	NFS_MAXID },
	{ NULL },	/* mfs */
	{ NULL },	/* msdos */
	{ NULL },	/* lfs */
	{ NULL },	/* ntfs */
	{ NULL },	/* fdesc */
	{ NULL },	/* portal */
	{ NULL },	/* null */
	{ NULL },	/* umap */
	{ NULL },	/* kern */
	{ NULL },	/* proc */
	{ NULL },	/* udf */
	{ NULL },	/* cd9660 */
	{ NULL },	/* union */
	{ NULL },	/* ados */
	{ NULL },	/* ext2 */
	{ NULL },	/* 18 */
	{ NULL },	/* 19 */
	{ NULL },	/* 20 */
	{ NULL },	/* xfs */
};

const struct ctlist rootlists[] = {
	{ NULL },
	{ kern,	kernlists,	KERN_MAXID },
	{ vm,	vmlists,	VM_MAXID },
	{ fs,	fslists,	FS_MAXID },
	{ net,	netlists,	NET_MAXID },
	{ debug,NULL,		0 },
	{ hw,	NULL,		HW_MAXID },
	{ md,	NULL,		CPU_MAXID },
	{ user,	NULL,		USER_MAXID },
	{ ddb,	NULL,		DBCTL_MAXID },
	{ vfs,	vfslists,	sizeof vfslists / sizeof vfslists[0] }
};
struct ctlist rootlist = { root, rootlists, CTL_MAXID };

const char *
sysctlname(int *name, u_int namelen, char *buf, size_t buflen)
{
	const struct ctlist *l;
	const char *p;
	char nbuf[16], *q;
	int i;

	if (kmems[0].ctl_name == NULL)
		for (i = 0; i < M_LAST; i++) {
			if (kmemnames[i] &&
			    (q = kmems[i].ctl_name = strdup(kmemnames[i])))
				for (; *q; q++)
					if (*q == ' ')
						*q = '_';
		}

	if (emul[0].ctl_type == 0) {
		int mib[4];
		size_t len;

		mib[0] = CTL_KERN;
		mib[1] = KERN_EMUL;
		mib[2] = KERN_EMUL_NUM;
		len = sizeof(int);
		if (sysctl(mib, 3, &i, &len, NULL, 0) < 0)
			emul[0].ctl_type = -1;
		else {
			while (i--) {
				mib[2] = i + 1;
				mib[3] = KERN_EMUL_NAME;
				len = sizeof(nbuf);
				if (sysctl(mib, 4, nbuf, &len, NULL, 0) < 0)
					continue;
				emul[i+1].ctl_name = strdup(nbuf);
			}
		}
	}

	buf[0] = '\0';
	for (l = &rootlist, i = 0; i < namelen; name++, i++) {

		if (i)
			strlcat(buf, ".", buflen);

		if (!l || !l->names || *name >= l->maxid ||
		    !l->names[*name].ctl_name) {
			snprintf(nbuf, sizeof nbuf, "%d", *name);
			p = nbuf;
		} else
			p = l->names[*name].ctl_name;

		if (l && l->lists)
			l = &l->lists[l->maxid == -1? 0 : *name];
		else
			l = NULL;

		strlcat(buf, p, buflen);
	}

	return buf;
}
