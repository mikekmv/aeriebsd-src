
LIB=	l
SRCS=	libmain.c libyywrap.c
NOPIC=

.PATH:	${.CURDIR}/../../usr.bin/lex

.include <bsd.own.mk>

LINKS=	${LIBDIR}/libl.a ${LIBDIR}/libfl.a
.if !defined(NOPROFILE)
LINKS+=	${LIBDIR}/libl_p.a ${LIBDIR}/libfl_p.a
.endif

.include <bsd.lib.mk>
