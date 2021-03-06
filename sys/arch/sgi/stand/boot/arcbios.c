/*-
 * Copyright (c) 1996 M. Warner Losh.  All rights reserved.
 * Copyright (c) 1996-2004 Opsycon AB.  All rights reserved.
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
#include <lib/libkern/libkern.h>
#include <machine/autoconf.h>
#include <mips64/arcbios.h>
#include <mips64/archtype.h>
#include <stand.h>

#define	USE_SGI_PARTITIONS	1

int	bios_is_32bit;

void	bios_configure_memory(void);
int	bios_get_system_type(void);
const char *bios_get_path_component(const char *, char *, int *);

static struct systypes {
	char *sys_vend;		/* Vendor ID if name is ambiguous. */
	char *sys_name;		/* May be left NULL if name is sufficient. */
	int  sys_type;
} sys_types[] = {
    { NULL,		"PICA-61",			ACER_PICA_61 },
    { NULL,		"NEC-R94",			ACER_PICA_61 },
    { NULL,		"DESKTECH-TYNE",		DESKSTATION_TYNE },
    { NULL,		"DESKTECH-ARCStation I",	DESKSTATION_RPC44 },
    { NULL,		"Microsoft-Jazz",		MAGNUM },
    { NULL,		"RM200PCI",			SNI_RM200 },
    { NULL,		"SGI-IP17",			SGI_CRIMSON },
    { NULL,		"SGI-IP19",			SGI_ONYX },
    { NULL,		"SGI-IP20",			SGI_INDIGO },
    { NULL,		"SGI-IP21",			SGI_POWER },
    { NULL,		"SGI-IP22",			SGI_INDY },
    { NULL,		"SGI-IP25",			SGI_POWER10 },
    { NULL,		"SGI-IP26",			SGI_POWERI },
    { NULL,		"SGI-IP27",			SGI_O200 },
    { NULL,		"SGI-IP30",			SGI_OCTANE },
    { NULL,		"SGI-IP32",			SGI_O2 }
};

#define KNOWNSYSTEMS (sizeof(sys_types) / sizeof(struct systypes))

/*
 * ARCBios trampoline code.
 */
#define ARC_Call(Name,Offset)	\
__asm__("\n"			\
"	.text\n"		\
"	.ent	" #Name "\n"	\
"	.align	3\n"		\
"	.set	noreorder\n"	\
"	.globl	" #Name "\n"	\
#Name":\n"			\
"	lw	$2, 0xffffffff80001020\n"\
"	lw	$2," #Offset "($2)\n"\
"	jr	$2\n"		\
"	nop\n"			\
"	.end	" #Name "\n"	);

ARC_Call(Bios_Load,			0x00);
ARC_Call(Bios_Invoke,			0x04);
ARC_Call(Bios_Execute,			0x08);
ARC_Call(Bios_Halt,			0x0c);
ARC_Call(Bios_PowerDown,		0x10);
ARC_Call(Bios_Restart,			0x14);
ARC_Call(Bios_Reboot,			0x18);
ARC_Call(Bios_EnterInteractiveMode,	0x1c);
ARC_Call(Bios_Unused1,			0x20);
ARC_Call(Bios_GetPeer,			0x24);
ARC_Call(Bios_GetChild,			0x28);
ARC_Call(Bios_GetParent,		0x2c);
ARC_Call(Bios_GetConfigurationData,	0x30);
ARC_Call(Bios_AddChild,			0x34);
ARC_Call(Bios_DeleteComponent,		0x38);
ARC_Call(Bios_GetComponent,		0x3c);
ARC_Call(Bios_SaveConfiguration,	0x40);
ARC_Call(Bios_GetSystemId,		0x44);
ARC_Call(Bios_GetMemoryDescriptor,	0x48);
ARC_Call(Bios_Unused2,			0x4c);
ARC_Call(Bios_GetTime,			0x50);
ARC_Call(Bios_GetRelativeTime,		0x54);
ARC_Call(Bios_GetDirectoryEntry,	0x58);
ARC_Call(Bios_Open,			0x5c);
ARC_Call(Bios_Close,			0x60);
ARC_Call(Bios_Read,			0x64);
ARC_Call(Bios_GetReadStatus,		0x68);
ARC_Call(Bios_Write,			0x6c);
ARC_Call(Bios_Seek,			0x70);
ARC_Call(Bios_Mount,			0x74);
ARC_Call(Bios_GetEnvironmentVariable,	0x78);
ARC_Call(Bios_SetEnvironmentVariable,	0x7c);
ARC_Call(Bios_GetFileInformation,	0x80);
ARC_Call(Bios_SetFileInformation,	0x84);
ARC_Call(Bios_FlushAllCaches,		0x88);
ARC_Call(Bios_TestUnicodeCharacter,	0x8c);
ARC_Call(Bios_GetDisplayStatus,		0x90);

/*
 * Simple getchar/putchar interface.
 */

int
getchar()
{
	char buf[4];
	int cnt;

	if (Bios_Read(0, &buf[0], 1, &cnt) != 0)
		return (-1);

	return (buf[0] & 255);
}

void
putchar(int c)
{
	char buf[4];
	int cnt;

	if (c == '\n') {
		buf[0] = '\r';
		buf[1] = c;
		cnt = 2;
	} else {
		buf[0] = c;
		cnt = 1;
	}

	Bios_Write(1, &buf[0], cnt, &cnt);
}

/*
 * Identify system type.
 */
int
bios_get_system_type()
{
	arc_config_t *cf;
	arc_sid_t *sid;
	char *sysid;
	int i, sysid_len;

	/*
	 * Figure out if this is an ARCBios machine and if it is, see if we're
	 * dealing with a 32 or 64 bit version.
	 */
	if ((ArcBiosBase32->magic == ARC_PARAM_BLK_MAGIC) ||
	    (ArcBiosBase32->magic == ARC_PARAM_BLK_MAGIC_BUG)) {
		bios_is_32bit = 1;
		printf("ARCS32 Firmware Version %d.%d\n",
		    ArcBiosBase32->version, ArcBiosBase32->revision);
	} else if ((ArcBiosBase64->magic == ARC_PARAM_BLK_MAGIC) ||
	    (ArcBiosBase64->magic == ARC_PARAM_BLK_MAGIC_BUG)) {
		bios_is_32bit = 0;
		printf("ARCS64 Firmware Version %d.%d\n",
		    ArcBiosBase64->version, ArcBiosBase64->revision);
	} else
		return (-1);	/* XXX BAD BAD BAD!!! */

	sid = (arc_sid_t *)Bios_GetSystemId();

	cf = (arc_config_t *)Bios_GetChild(NULL);
	if (cf != NULL) {
		if (bios_is_32bit) {
			sysid = (char *)(long)cf->id;
			sysid_len = cf->id_len;
		} else {
			sysid = (char *)((arc_config64_t *)cf)->id;
			sysid_len = ((arc_config64_t *)cf)->id_len;
		}

		if (sysid_len > 0 && sysid != NULL) {
			sysid_len--;
			for (i = 0; i < KNOWNSYSTEMS; i++) {
				if (strlen(sys_types[i].sys_name) !=sysid_len)
					continue;
				if (strncmp(sys_types[i].sys_name, sysid,
				    sysid_len) != 0)
					continue;
				if (sys_types[i].sys_vend &&
				    strncmp(sys_types[i].sys_vend, sid->vendor,
				      8) != 0)
					continue;
				return (sys_types[i].sys_type);	/* Found it. */
			}
		}
	} else {
#if defined(TGT_ORIGIN200) || defined(TGT_ORIGIN2000)
		if (IP27_KLD_KLCONFIG(0)->magic == IP27_KLDIR_MAGIC) {
			/* If we find a kldir assume IP27. */
			return (SGI_O200);
		}
#endif
	}

	printf("UNRECOGNIZED SYSTEM '%s' VENDOR '%8.8s' PRODUCT '%8.8s'\n",
	    cf == NULL ? "??" : sysid, sid->vendor, sid->prodid);
	printf("See www.openbsd.org for further information.\n");
	printf("Halting system!\n");
	Bios_Halt();
	printf("Halting failed, use manual reset!\n");
	while (1);
}

/*
 *  Decompose the device pathname and find driver.
 *  Returns pointer to remaining filename path in file.
 */
int
devopen(struct open_file *f, const char *fname, char **file)
{
	const char *cp, *ncp, *ecp;
	struct devsw *dp;
	int partition = 0;
	char namebuf[256];
	char devname[32];
	int rc, i, n;

	ecp = cp = fname;

	/*
	 * Scan the component list and find device and partition.
	 */
	while ((ncp = bios_get_path_component(cp, namebuf, &i)) != NULL) {
		if (strcmp(namebuf, "partition") == 0) {
			partition = i;
			if (USE_SGI_PARTITIONS)
				ecp = ncp;
		} else
			ecp = ncp;

		/* XXX Do this with a table if more devs are added. */
		if (strcmp(namebuf, "scsi") == 0)
			strncpy(devname, namebuf, sizeof(devname)); 

		cp = ncp;
	}

	memcpy(namebuf, fname, ecp - fname);
	namebuf[ecp - fname] = '\0';

	/*
	 * Dig out the driver.
	 */
	dp = devsw;
	n = ndevs;
	while(n--) {
		if (strcmp (devname, dp->dv_name) == 0) {
			rc = (dp->dv_open)(f, namebuf, partition, 0);
			if (!rc) {
				f->f_dev = dp;
				if (file && *cp != '\0')
					*file = (char *)cp;
			}
			return (rc);
		}
		dp++;
	}
	return (ENXIO);
}

const char *
bios_get_path_component(const char *p, char *comp, int *no)
{
	while (*p && *p != '(')
		*comp++ = *p++;
	*comp = '\0';

	if (*p == NULL)
		return (NULL);

	*no = 0;
	p++;
	while (*p && *p != ')') {
		if (*p >= '0' && *p <= '9')
			*no = *no * 10 + *p++ - '0';
		else
			return (NULL);
	}
	return (++p);
}
