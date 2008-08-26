
.include <bsd.own.mk>

.if (${ELF_TOOLCHAIN:L} == "no")
PROG=	ar
CFLAGS+=-I${.CURDIR}
SRCS=	append.c ar.c archive.c contents.c delete.c extract.c misc.c \
	move.c print.c replace.c 
MAN=	ar.1
.else
NOPROG=
.endif

MAN+=	ar.5

.include <bsd.prog.mk>
