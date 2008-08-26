
CFLAGS+=-Wall -Wmissing-prototypes -Wshadow
LDADD+= -lpcap -lutil
DPADD+=	${LIBPCAP} ${LIBUTIL}

PROG=	pflogd
SRCS=	pflogd.c privsep.c privsep_fdpass.c
MAN=	pflogd.8

.include <bsd.prog.mk>
