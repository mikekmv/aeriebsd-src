#	@(#)Makefile	4.7 (Berkeley) 3/1/91

PROG=	tbl
CFLAGS+=-g -Wall
SRCS=	tbl.c utils.c

.if make(install)
SUBDIR+= USD.doc
.endif

.include <bsd.prog.mk>
