KRBDEPTH=../..
.include <${.CURDIR}/../../Makefile.inc>

PROG=	kpasswdd
MAN=	${PROG}.8

SRCS=	kpasswdd.c
SRCS+=	${PRINTVER}

.PATH:	${KRB5SRC}/kpasswd

CFLAGS+=-I${KRB5SRC}/lib/krb5 -I${KRB5SRC}/lib/kadm5

DPADD+=	${LIBKADM5SRV} ${LIBKRB5} ${LIBCRYPTO} ${LIBUTIL}
LDADD+=	-lkadm5srv -lkrb5 -lcrypto -lutil

.include <bsd.prog.mk>
