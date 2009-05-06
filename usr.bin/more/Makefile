#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	more
CPPFLAGS+=-I${.CURDIR}
CFLAGS+=-Wall
CFLAGS+=-Wstrict-prototypes -Wmissing-prototypes
CFLAGS+=-Wmissing-declarations
CFLAGS+=-Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=-Wsign-compare
SRCS=	ch.c command.c decode.c input.c line.c linenum.c main.c \
	output.c prim.c screen.c tags.c
DPADD=	${LIBTERM}
LDADD=	-ltermcap

beforeinstall:
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 ${.CURDIR}/more.help \
	    ${DESTDIR}/usr/share/misc

.include <bsd.prog.mk>
