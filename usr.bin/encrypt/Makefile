
# Must be static because the floppies use it through a chroot.

LDSTATIC=	${STATIC}
PROG=	encrypt
SRCS= encrypt.c pwd_gensalt.c
.PATH: ${.CURDIR}/../passwd

LDADD = -lutil
DPADD= ${LIBUTIL}
LIBEXEC?=       /usr/libexec

LINKS=	${BINDIR}/encrypt ${LIBEXEC}/makekey
MLINKS=	encrypt.1 makekey.8

.include <bsd.prog.mk>
