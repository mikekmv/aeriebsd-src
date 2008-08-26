
PROG=	lint1
SRCS=	cgram.y scan.l mem1.c mem.c err.c main1.c decl.c tree.c func.c \
	init.c emit.c emit1.c
NOMAN=
LDADD+=	-ll
DPADD+=	${LIBL}
YFLAGS=	-d
CFLAGS+=-I. -I${.CURDIR}
LINTFLAGS=-aehpz

BINDIR=	/usr/libexec

.include <bsd.prog.mk>
