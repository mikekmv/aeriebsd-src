
PROG=	init
MAN=
COPTS+=-Os
CFLAGS+=-DDEFAULT_STATE=single_user
DPADD=	${LIBUTIL}
LDADD=	-lutil
.PATH:  ${.CURDIR}/../../../sbin/init

.include <bsd.prog.mk>
