
NOMAN=
PROG=strerror_test
CPPFLAGS+=-I${.CURDIR}/../../../../lib/libc
CLEANFILES+= invalid.out valid.out

run-regress-strerror_test: ${PROG}
	./${PROG} >${.OBJDIR}/valid.out
	cmp -s ${.OBJDIR}/valid.out ${.CURDIR}/valid.ok

.include <bsd.regress.mk>
