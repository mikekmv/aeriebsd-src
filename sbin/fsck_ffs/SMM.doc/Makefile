
DIR=	smm/03.fsck_ffs
SRCS=	0.t 1.t 2.t 3.t 4.t
MACROS=	-ms

paper.txt: ${SRCS}
	${ROFF} -Tascii ${SRCS} > ${.TARGET}

.include <bsd.doc.mk>
