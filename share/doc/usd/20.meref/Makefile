

DIR=	usd/20.meref
SRCS=	ref.me
MACROS=	-me

paper.txt: ${SRCS}
	${ROFF} -Tascii ${SRCS} > ${.TARGET}

.include <bsd.doc.mk>
