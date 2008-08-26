
PROG=	pcap

DPADD+=	${LIBPCAP}
LDADD+=	-lpcap

REGRESS_ROOT_TARGETS=	run-regress-${PROG}

run-regress-${PROG}: ${PROG}
	${SUDO} ./${PROG}

.include <bsd.regress.mk>
