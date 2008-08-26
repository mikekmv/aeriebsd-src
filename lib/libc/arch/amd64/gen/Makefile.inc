
SRCS+=	_setjmp.S fabs.S infinity.c isinf.c isnan.c ldexp.c \
	modf.S setjmp.S sigsetjmp.S
SRCS+=	flt_rounds.S fpgetmask.S fpgetround.S fpgetsticky.S fpsetmask.S \
	fpsetround.S fpsetsticky.S

SRCS+=	alloca.S
LSRCS+= alloca.c
