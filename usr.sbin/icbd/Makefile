PROG=	icbd
SRCS=	cmd.c icb.c icbd.c
MAN=	icbd.8

CFLAGS+=-Wall -Werror

DPADD=	${LIBEVENT}
LDADD=	-levent

.include <bsd.prog.mk>
