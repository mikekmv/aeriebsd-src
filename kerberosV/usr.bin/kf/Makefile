KRBDEPTH=../..
.include <${.CURDIR}/../../Makefile.inc>

PROG=	kf

SRCS=	kf.c

.PATH:	${KRB5SRC}/appl/kf

DPADD+= ${LIBKRB5} ${LIBDES} ${LIBCRYPTO}
LDADD+=	-lkrb5 -ldes -lcrypto

.include <bsd.prog.mk>
