
PROG=	itimer
SRCS=	itimer.c

real: itimer
	./itimer -r

virtual: itimer
	./itimer -v

prof: itimer
	./itimer -p

REGRESS_TARGETS=real virtual prof

.include <bsd.regress.mk>
