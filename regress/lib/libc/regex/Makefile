
PROG=	re
SRCS=	main.c split.c debug.c regcomp.c regerror.c regexec.c regfree.c
.PATH: ${.CURDIR}/../../../../lib/libc/regex

CFLAGS+= -I${.CURDIR}/../../../../lib/libc/regex -DREDEBUG -DPOSIX_MISTAKE

TESTS=	${.CURDIR}/tests

REGRESS_TARGETS=do-reg do-reg-long do-reg-backref

do-reg: ${PROG}
	./re < ${TESTS}
do-reg-long: ${PROG}
	./re -el < ${TESTS}
do-reg-backref: ${PROG}
	./re -er < ${TESTS}

.include <bsd.regress.mk>
