
struct _machdep_state {
	u_long	sp;
	u_long	fp;
	u_int64_t fpregs[32];
};
