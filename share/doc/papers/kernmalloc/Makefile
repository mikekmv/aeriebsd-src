

DIR=	papers/kernmalloc
SRCS=	kernmalloc.t appendix.t
MACROS=	-ms

paper.ps: ${SRCS} alloc.fig usage.tbl
	${SOELIM} ${SRCS} | ${TBL} | ${PIC} | ${EQN} | ${GRIND} | \
	    ${ROFF} > ${.TARGET}

paper.txt: ${SRCS} alloc.fig usage.tbl
	${SOELIM} ${SRCS} | ${TBL} | ${PIC} | ${EQN} | ${GRIND} | \
	    ${ROFF} -Tascii > ${.TARGET}

.include <bsd.doc.mk>
