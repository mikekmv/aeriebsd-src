#	@(#)Makefile	8.8 (Berkeley) 6/21/2000
#

PROG=   growfs
SRCS=   growfs.c
MAN=	growfs.8

#CFLAGS+=-Wall

DPADD=	${LIBUTIL}
LDADD=	-lutil

.if defined(GFSDBG)
SRCS+=  debug.c
CFLAGS+=-DFS_DEBUG
.endif  

.include <bsd.prog.mk>      
