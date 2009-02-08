
PROG=	nm
SRCS=	nm.c
CPPFLAGS+=-I${.CURDIR}
LDADD=	-lelf
DPADD=	${LIBELF}
LINKS=	${BINDIR}/nm ${BINDIR}/size
MAN=	nm.1 size.1

.include <bsd.prog.mk>
