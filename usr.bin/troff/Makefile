#	@(#)Makefile	5.5 (Berkeley) 5/11/90

PROG=	troff
SRCS=	ni.c nii.c n1.c n2.c n3.c n4.c n5.c t6.c n7.c n8.c t10.c suftab.c
CPPFLAGS+=-I${.CURDIR}/../nroff
CFLAGS+=-g -Wall
DPADD=	${LIBCOMPAT}
LDADD=	-lcompat

.PATH:	${.CURDIR}/../nroff

.include <bsd.prog.mk>
