
PROG=	sigret

DEBUG+= -ggdb

REGRESS_TARGETS+= sigret-normal sigret-indirect sigret-altstack

sigret-normal: ${PROG}
	./${PROG}

sigret-indirect: ${PROG}
	./${PROG} -i

sigret-altstack: ${PROG}
	./${PROG} -a

.include <bsd.regress.mk>
