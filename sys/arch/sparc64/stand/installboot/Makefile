
.include <bsd.own.mk>

MAN=	installboot.8
MANSUBDIR=sparc64

.if ${MACHINE} == "sparc64"
PROG=	installboot
SRCS=	installboot.c loadfile.c
.else
NOPROG=
.endif

LIBSA_I=${.CURDIR}/../../../../lib/libsa

.PATH.c: ${.CURDIR}/../../../../lib/libsa

CPPFLAGS+=-I${LIBSA_I} -I. -I${.CURDIR}/../../../.. -I${.OBJDIR}

LDSTATIC?=	-static

.if !make(obj)
.BEGIN:
	@([ -h machine ] || ln -s ${.CURDIR}/../../../${MACHINE}/include machine)
	@([ -h sparc ] || ln -s ${.CURDIR}/../../../sparc/include sparc)
.NOPATH: machine sparc
CLEANFILES+= machine sparc
.endif

STANDALONE=

.include <bsd.prog.mk>
