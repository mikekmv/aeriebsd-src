
MAN=	installboot.8

.if ${MACHINE} == "amd64"
PROG=	installboot
CPPFLAGFS+=-I${.CURDIR}

LDADD=	-lutil
DPADD=	${LIBUTIL}
# Need this to work in the miniroot
LDSTATIC=	-static

SADIR=	${.CURDIR}/..
.else
NOPROG=
.endif

.include <bsd.prog.mk>
