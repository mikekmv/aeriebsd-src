

DIR=	usd/18.msdiffs
SRCS=	ms.diffs
MACROS=	-ms

paper.txt: ${SRCS}
	${ROFF} -Tascii ${SRCS} > ${.TARGET}

.include <bsd.doc.mk>
