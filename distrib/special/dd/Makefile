
COPTS+=-Os
# Define NO_CONV to disable character conversion
CFLAGS+=-DNO_CONV
LDFLAGS=-static

PROG=	dd
SRCS=	args.c conv.c dd.c misc.c position.c
MAN=

.PATH:	${.CURDIR}/../../../bin/dd

.include <bsd.prog.mk>
