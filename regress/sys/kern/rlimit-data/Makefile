
PROG=	rlimit
REGRESS_TARGETS=rlimit-data rlimit-stack

rlimit-data: ${PROG}
	@./${PROG}

rlimit-stack: ${PROG}
	@./${PROG} s

.include <bsd.regress.mk>
