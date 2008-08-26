
PROG=	rdate
SRCS=	rdate.c rfc868time.c ntp.c ntpleaps.c
CFLAGS+=-Wall
DPADD+= ${LIBUTIL}
LDADD+= -lutil

MAN=	rdate.8

.include <bsd.prog.mk>
