
PROG=	faithd
SRCS=	faithd.c tcp.c ftp.c prefix.c

MAN=	faithd.8

#CPPFLAGS+= -DFAITH4

LDADD+=	-lutil
DPADD+=	${LIBUTIL}

.include <bsd.prog.mk>
