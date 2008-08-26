
.PATH:	${.CURDIR}/..

MAN=	installboot.8
MANSUBDIR=alpha

.if ${MACHINE} == "alpha"
PROG=	installboot
BINDIR=	/usr/mdec
DPADD+= ${LIBUTIL}
LDADD+= -lutil
LDSTATIC=-static
.else
NOPROG=
.endif

.include <bsd.prog.mk>
