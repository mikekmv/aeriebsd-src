
PROG=	rcp
SRCS=	rcp.c util.c
#INSTALLFLAGS=-fschg

.include <bsd.own.mk>	# For KERBEROS

#.if (${KERBEROS:L} == "yes")
#.PATH:	${.CURDIR}/../../usr.bin/rsh
#SRCS+=	krcmd.c kcmd.c
#CFLAGS+=-DKERBEROS
#DPADD+=	${LIBKRB}
#LDADD+=	-lkrb
#.endif

.include <bsd.prog.mk>
