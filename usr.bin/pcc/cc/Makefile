
PROG=  	cc
SRCS=	cc.c strlist.c xalloc.c

TARGOS=		aeriebsd
TARGMACH=       ${MACHINE_ARCH}

CPPFLAGS+=-I${.CURDIR}/.. -I${.CURDIR}/../${MACHINE_ARCH}
CPPFLAGS+=-DTARGOS=\"${TARGOS}\" -DTARGMACH=\"${TARGMACH}\"

.include <bsd.prog.mk>
.include "../../Makefile.inc"
