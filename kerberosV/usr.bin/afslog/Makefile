KRBDEPTH=../..
.include <${.CURDIR}/../../Makefile.inc>

PROG	= afslog

.PATH:	${KRB5SRC}/appl/afsutil

SRCS+=	afslog.c
SRCS+=	${PRINTVER}

DPADD	+= ${LIBKRB5} ${LIBDES} ${LIBCRYPTO}
LDADD	+= -lkrb5 -ldes -lcrypto

.include <bsd.prog.mk>
