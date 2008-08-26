
DIR=	smm/17.password
SRCS=	password.ms
MACROS=	-ms

paper.ps: ${SRCS}
	${TBL} ${SRCS} | ${EQN} | ${ROFF} > ${.TARGET}

paper.txt: ${SRCS}
	${TBL} ${SRCS} | ${EQN} -Tascii | ${ROFF} -Tascii > ${.TARGET}

.include <bsd.doc.mk>
