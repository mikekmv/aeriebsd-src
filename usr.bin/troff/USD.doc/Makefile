#	@(#)Makefile	8.2 (Berkeley) 5/16/94

DIR=	usd/21.troff
SRCS=	m0 m0a m1 m2 m3 m4 m5
APNDX=	table1 table2
MACROS= m.mac -

all: paper.ps apndx.ps

paper.ps: ${SRCS}
	${TBL} ${SRCS} | ${ROFF} > ${.TARGET}

apndx.ps: ${APNDX}
	${TBL} ${APNDX} | ${ROFF} > ${.TARGET}

paper.txt: ${SRCS}
        ${TBL} ${SRCS} | ${ROFF} -Tascii > ${.TARGET}

clean:
	rm -f paper.ps apndx.ps *.spell errs Errs make.out

spell: ${SRCS} ${APNDX}
	@for i in ${PAPER} ${APNDX}; do \
		echo $$i; spell $$i | sort | comm -23 - spell.ok > $$i.spell; \
	done

.include <bsd.doc.mk>
