#	from: @(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	lpq
CFLAGS+=-I${.CURDIR}/../common_source
SRCS=	lpq.c displayq.c common.c common_vars.c
BINGRP=	daemon
BINMODE=2555
BINDIR=	/usr/bin
.PATH:	${.CURDIR}/../common_source

.include <bsd.prog.mk>
