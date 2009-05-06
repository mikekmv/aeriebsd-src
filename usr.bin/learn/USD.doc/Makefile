#	@(#)Makefile	8.2 (Berkeley) 5/21/94

DIR=	usd/02.learn
SRCS=	p0 p1 p2 p3 p4 p5 p6 p7
MACROS=	-ms
REFOPTS=-p Refs -p /usr/share/dict/papers/Ind -e

paper.ps: ${SRCS}
	${REFER} -p Refs -p /usr/old/dict/papers/Ind -e ${SRCS} | \
		${TBL} | ${ROFF} > ${.TARGET}

paper.ps: ${SRCS}
	${REFER} ${REFOPTS} ${SRCS} | ${TBL} | ${ROFF} > ${.TARGET}

paper.txt: ${SRCS}
	${REFER} ${REFOPTS} ${SRCS} | ${TBL} -Tascii | \
	    ${ROFF} -Tascii > ${.TARGET}

.include <bsd.doc.mk>
