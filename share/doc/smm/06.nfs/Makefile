

DIR=	smm/06.nfs
SRCS=	0.t 1.t 2.t ref.t
MACROS=	-me

paper.txt: ${SRCS}
	${ROFF} -Tascii ${SRCS} > ${.TARGET}

.include <bsd.doc.mk>
