
PROG=	rtsol
SRCS=	rtsold.c rtsol.c if.c probe.c

CPPFLAGS+=-DSMALL

#MAN=	rtsold.8
NOMAN=	yes

LDADD+=	-lutil
DPADD+=	${LIBUTIL}

.PATH:	${.CURDIR}/../../usr.sbin/rtsold

.include <bsd.prog.mk>
