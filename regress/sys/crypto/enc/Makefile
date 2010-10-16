
PROG=   des3
LDADD=-lcrypto
DPADD=${LIBCRYPTO}

REGRESS_ROOT_TARGETS=	run-regress-${PROG}

run-regress-${PROG}: ${PROG}
	${SUDO} ./${PROG}

.include <bsd.regress.mk>
