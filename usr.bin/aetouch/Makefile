PROG=	aetouch
SRCS=	aetouch.c attach.c
MAN=	aetouch.1
CFLAGS+=-Wall -Werror

DPADD=	${LIBUTIL} ${LIBEVENT}
LDADD=	-lutil -levent

.include <bsd.prog.mk>
