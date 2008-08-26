
PROG=	spamlogd
SRCS=	spamlogd.c sync.c
MAN=	spamlogd.8

CFLAGS+= -Wall -Wstrict-prototypes -I${.CURDIR}/../spamd
LDADD+= -lpcap -lcrypto
DPADD+=	${LIBPCAP}
.PATH:  ${.CURDIR}/../spamd

.include <bsd.prog.mk>
