
PROG=	infinity

DPADD+=	${LIBM}
LDADD+=	-lm

REGRESS_TARGETS+= add mult neg pumpkin

add: ${PROG}
	./${PROG} -a

mult: ${PROG}
	./${PROG} -m

neg: ${PROG}
	./${PROG} -n

pumpkin: ${PROG}
	./${PROG} -p

.include <bsd.regress.mk>
