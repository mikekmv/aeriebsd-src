KRBDEPTH=../..
.include <${.CURDIR}/../../Makefile.inc>

PROG=	kfd
MAN=	${PROG}.8

SRCS=	kfd.c

.PATH:	${KRB5SRC}/appl/kf

CFLAGS+=-I${KRB5SRC}/lib/krb5

DPADD+=	${LIBKRB5} ${LIBCRYPTO}
LDADD+=	-lkrb5 -lcrypto

.include <bsd.prog.mk>
