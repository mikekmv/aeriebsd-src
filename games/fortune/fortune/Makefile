
PROG=		fortune
MAN=		fortune.6
CFLAGS+=	-I${.CURDIR}/../strfile 
LDADD+=		-lcompat
DPADD+=		${LIBCOMPAT}

.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.prog.mk>
