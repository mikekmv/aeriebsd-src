KRBDEPTH=../..
.include <${.CURDIR}/../../Makefile.inc>

PROG=	kstash
MAN=	${PROG}.8

SRCS=	kstash.c \
	sl.c
SRCS+=	${PRINTVER}

.PATH:	${KRB5SRC}/kdc
.PATH:	${KRB5SRC}/lib/sl

CFLAGS+=-I${KRB5SRC}/lib/krb5

DPADD+= ${LIBEDIT} ${LIBCURSES} ${LIBKRB5} ${LIBCRYPTO}
LDADD+=	-ledit -lcurses -lkrb5 -lcrypto

.include <bsd.prog.mk>
