
PROG=	savecore

SRCS=	savecore.c zopen.c
CFLAGS+=-DSAVECORE
LDADD=	-lkvm
DPADD=	${LIBKVM}
MAN=	savecore.8
.PATH:	${.CURDIR}/../../usr.bin/compress

.include <bsd.prog.mk>
