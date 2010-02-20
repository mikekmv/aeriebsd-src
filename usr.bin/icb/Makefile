PROG=	icb
SRCS=	icb.c screen.c
MAN=	icb.1

CFLAGS+=-Wall -Werror

DPADD=	${LIBTERMCAP} ${LIBEVENT}
LDADD=	-ltermcap -levent

.include <bsd.prog.mk>
