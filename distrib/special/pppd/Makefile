
COPTS+=-Os

PROG=	pppd
SRCS=	auth.c cbcp.c ccp.c chap.c chap_ms.c demand.c fsm.c ipcp.c \
	lcp.c magic.c main.c options.c sys-bsd.c upap.c

.PATH:	${.CURDIR}/../../../usr.sbin/pppd ${.CURDIR}/../../sys/net
MAN=
BINMODE=4555
BINOWN=	root

LDADD=	-lutil
DPADD=	${LIBUTIL}
CFLAGS+= -I. -DHAVE_PATHS_H

.include <bsd.prog.mk>
