

DIR=	papers/nqnfs
SRCS=	nqnfs.me
MACROS=	-me

paper.ps: ${SRCS}
	${PIC} ${SRCS} | ${TBL} | ${ROFF} > ${.TARGET}

paper.txt: ${SRCS}
	${PIC} ${SRCS} | ${TBL} | ${ROFF} -Tascii > ${.TARGET}

.include <bsd.doc.mk>
