
MAN=pctr.1

CFLAGS+=-W -Wall -Werror

.if ${MACHINE} == "amd64" || ${MACHINE} == "i386"
PROG=	pctr
.else
NOPROG=
.endif

.include <bsd.prog.mk>
