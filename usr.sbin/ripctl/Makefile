
.PATH:		${.CURDIR}/../ripd

PROG=	ripctl
SRCS=	buffer.c imsg.c log.c ripctl.c parser.c
CFLAGS+= -Wall
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations
CFLAGS+= -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare
CFLAGS+= -I${.CURDIR} -I${.CURDIR}/../ripd
MAN=	ripctl.8

.include <bsd.prog.mk>
