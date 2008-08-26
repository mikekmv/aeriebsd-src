
DIR=	usd/05.dc
SRCS=	dc
MACROS=	-ms

paper.ps: ${SRCS}
	${EQN} ${SRCS} | ${ROFF} > ${.TARGET}

paper.txt: ${SRCS}
	${EQN} -Tascii ${SRCS} | ${ROFF} -Tascii > ${.TARGET}

.include <bsd.doc.mk>
