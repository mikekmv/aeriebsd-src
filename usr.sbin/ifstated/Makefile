
PROG=	ifstated
SRCS=	ifstated.c log.c parse.y
CFLAGS+= -Wall -I${.CURDIR}
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations -Wredundant-decls
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
YFLAGS=
MAN= ifstated.8 ifstated.conf.5
LDADD+=-levent
DPADD+= ${LIBEVENT}

.include <bsd.prog.mk>
