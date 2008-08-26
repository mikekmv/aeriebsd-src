
PROG=	rarpd
SRCS=	rarpd.c arptab.c
MAN=	rarpd.8
LDADD+=	-lutil
DPADD+=	${LIBUTIL}

# Uncomment the following to require a boot file in TFTP_DIR
# to exist for a rarp reply to be sent.
# CFLAGS+=-DREQUIRE_TFTPBOOT

.include <bsd.prog.mk>
