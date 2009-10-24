#	@(#)Makefile	5.8 (Berkeley) 7/18/92

PROG=	eqn
CPPFLAGS+=-I. -I${.CURDIR}
CFGALS+=-g
YFLAGS+=-v
SRCS=	e.y io.c lex.c lookup.c text.c
LINKS=  ${BINDIR}/eqn ${BINDIR}/neqn
MAN=	eqn.1eqn
MLINKS=	eqn.1 checkeq.1 eqn.1 neqn.1
CLEANFILES+=y.tab.h

.include <bsd.prog.mk>
