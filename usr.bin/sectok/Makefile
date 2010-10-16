
PROG=	sectok
SRCS=   main.c cmds.c cyberflex.c
DPADD=	${LIBSECTOK} ${LIBCRYPTO}
LDADD=  -lsectok -lcrypto

CFLAGS+=-Wall

.include <bsd.prog.mk>
