
DIR=	usd/01.begin
SRCS=	u.mac u0 u1 u2 u3 u4 u5
MACROS= -ms

paper.txt: ${SRCS}
	${ROFF} -Tascii ${SRCS} > ${.TARGET}

.include <bsd.doc.mk>
