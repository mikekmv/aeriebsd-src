
.if ${MACHINE} == "i386" || ${MACHINE} == "amd64" || \
    ${MACHINE} == "alpha" || ${MACHINE} == "hppa" || \
    ${MACHINE} == "zaurus"

PROG=	wsfontload
SRCS=	wsfontload.c

.else

NOPROG=yes

.endif

MAN=	wsfontload.8
MANSUBDIR=	alpha hppa i386 amd64 zaurus

.include <bsd.prog.mk>
