
PROG=	netstat
SRCS=	if.c inet.c inet6.c main.c mbuf.c mroute.c route.c \
	unix.c atalk.c mroute6.c net80211.c show.c
BINGRP=	kmem
BINMODE=2555
LDADD=	-lkvm -lutil
DPADD=	${LIBKVM} ${LIBUTIL}

.include <bsd.prog.mk>
