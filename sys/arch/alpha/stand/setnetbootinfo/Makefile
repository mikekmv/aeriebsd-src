
.PATH:	${.CURDIR}/..

MAN=	setnetbootinfo.8
MANSUBDIR=alpha

.if ${MACHINE} == "alpha"
PROG=	setnetbootinfo
BINDIR=	/usr/mdec

CFLAGS+= -I${.CURDIR}/..
LDSTATIC=-static
.else
NOPROG=
.endif

.include <bsd.prog.mk>
