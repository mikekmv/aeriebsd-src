
DIR=	usd/09.edtut
SRCS=	e.mac e0 e1 e2 e3 e4 e5 e6 e7
MACROS= -ms

paper.txt: ${SRCS}
	${ROFF} -Tascii ${SRCS} > ${.TARGET}

.include <bsd.doc.mk>
