
.PATH:		${.CURDIR}/../ospfd

PROG=	ospfctl
SRCS=	buffer.c imsg.c log.c ospfctl.c parser.c
CFLAGS+= -Wall
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
CFLAGS+= -I${.CURDIR} -I${.CURDIR}/../ospfd
MAN=	ospfctl.8

.include <bsd.prog.mk>
