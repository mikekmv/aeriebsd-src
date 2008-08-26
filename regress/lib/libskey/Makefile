
PROG=	skeytest
CFLAGS+=-Wall
LDADD+=	-lskey
DPADD+=	${LIBSKEY}

skey-regress: skeytest
	./${PROG}

REGRESS_TARGETS=skey-regress
.PHONY: ${REGRESS_TARGETS}

.include <bsd.regress.mk>

