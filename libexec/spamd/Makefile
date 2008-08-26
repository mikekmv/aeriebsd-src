
PROG=	spamd
SRCS=	spamd.c sdl.c grey.c sync.c
MAN=	spamd.8

CFLAGS+= -Wall -Wstrict-prototypes

LDADD+=	-lcrypto
DPADD+=	${LIBCRYPTO}

.include <bsd.prog.mk>
