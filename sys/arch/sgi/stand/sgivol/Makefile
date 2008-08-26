
MAN=	sgivol.8
MANSUBDIR=sgi

.if ${MACHINE} == "sgi"
PROG=	sgivol
LDADD+=	-lutil
LDSTATIC=-static

.else
NOPROG=
.endif

.include <bsd.prog.mk>
