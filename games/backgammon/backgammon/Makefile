#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	backgammon
CFLAGS+=-I${.CURDIR}/../common_source
SRCS=	allow.c board.c check.c extra.c fancy.c init.c main.c move.c \
	odds.c pubeval.c one.c save.c subs.c table.c text.c
MAN=	backgammon.6
DPADD=	${LIBCURSES}
LDADD=	-lcurses
.PATH:	${.CURDIR}/../common_source

.include "../../Makefile.inc"
.include <bsd.prog.mk>
