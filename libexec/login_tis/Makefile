
PROG=	login_tis
MAN=	login_tis.8
CFLAGS+=-Wall
LDADD+=	-lcrypto
DPADD+=	${LIBCRYPTO}

BINOWN=	root
BINGRP=	auth
BINMODE=4555
BINDIR=	/usr/libexec/auth

.include <bsd.prog.mk>
