
.PATH:		${.CURDIR}/../dvmrpd

PROG=	dvmrpctl
SRCS=	buffer.c imsg.c log.c dvmrpctl.c parser.c

CFLAGS+= -Wall
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
CFLAGS+= -I${.CURDIR} -I${.CURDIR}/../dvmrpd

MAN=	dvmrpctl.8

.include <bsd.prog.mk>
