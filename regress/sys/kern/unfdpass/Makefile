
PROG=	unfdpass

CLEANFILES+=file1 file2 output test-sock
REGRESS_TARGETS=do-unfdpass1 do-unfdpass2

do-unfdpass1: ${PROG}
	./unfdpass > output
	cmp -s ${.CURDIR}/expected output

do-unfdpass2: ${PROG}
	./unfdpass -p > output
	cmp -s ${.CURDIR}/expected output

.include <bsd.regress.mk>
