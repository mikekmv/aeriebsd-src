
PROG=	rshd
SRCS=	rshd.c
MAN=	rshd.8
CFLAGS+= -Wall -Wno-unused

.include <bsd.own.mk>

#.if (${KERBEROS:L} == "yes")
#SRCS+=	des_rw.c
#.PATH:	${.CURDIR}/../../usr.bin/rsh
#CFLAGS+=-DKERBEROS
#DPADD+=	${LIBKRB}
#LDADD+=	-lkrb
#.endif

.include <bsd.prog.mk>
