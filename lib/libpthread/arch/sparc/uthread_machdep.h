/* David Leonard, <d@csee.uq.edu.au>. Public domain. */

struct _machdep_state {
	int	fp;			/* frame pointer */
	int	pc;			/* program counter */

	u_int32_t	fs_csr;		/* FP control/status */
	u_int32_t	fs_enabled;	/* enabled? */
	u_int64_t	fs_regs[16];	/* 16 64bit registers */
};
