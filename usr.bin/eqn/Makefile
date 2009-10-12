#	@(#)Makefile	5.8 (Berkeley) 7/18/92

PROG=	eqn
CFLAGS+=-I. -I${.CURDIR}
SRCS=	diacrit.c e.y eqnbox.c font.c fromto.c funny.c glob.c integral.c io.c \
	lex.c lookup.c mark.c matrix.c move.c over.c paren.c pile.c shift.c \
	size.c sqrt.c text.c
YFLAGS+=-d
MAN=	eqn.1eqn
MLINKS=	eqn.1 checkeq.1 eqn.1 neqn.1
CLEANFILES+=y.tab.h

.include <bsd.prog.mk>
