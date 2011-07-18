
PROG=   	cc
TARGOS=		aeriebsd
TARGMACH=       ${MACHINE_ARCH}

CPPFLAGS+=-I${.CURDIR}/.. -I${.CURDIR}/../${MACHINE_ARCH}
CPPFLAGS+=-DTARGOS=\"${TARGOS}\" -DTARGMACH=\"${TARGMACH}\"
CPPFLAGS+=-DINCLUDEDIR=\"/usr/include/\"

.include <bsd.prog.mk>
.include "../../Makefile.inc"
