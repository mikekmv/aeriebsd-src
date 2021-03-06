/* public domain */

/* Keep this define consistent with VM_PHYSSEG_MAX in <machine/vmparam.h> */
#define	NPHYS_RAM_SEGS	1

typedef struct cpu_kcore_hdr {
	vaddr_t		sysmap;
	phys_ram_seg_t	ram_segs[NPHYS_RAM_SEGS];
} cpu_kcore_hdr_t;
