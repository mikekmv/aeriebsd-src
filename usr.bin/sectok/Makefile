
PROG=	sectok
SRCS=   main.c cmds.c cyberflex.c
DPADD=	${LIBSECTOK} ${LIBCRYPTO} ${LIBDES}
LDADD=  -lsectok -lcrypto -ldes

CFLAGS+=-Wall

.include <bsd.prog.mk>
