#
# Makefile for the cc part of pcc.
#
PROG=   	cc
TARGOS=		aeriebsd
TARGMACH=       ${MACHINE_ARCH}

CPPFLAGS+=	-I${.CURDIR}/.. -I${.CURDIR}/../${MACHINE_ARCH}
CPPFLAGS+=	-DDEFMACH=\"${TARGMACH}\" -DINCLUDEDIR=\"/usr/include/\"

.include <bsd.prog.mk>
.include "../../Makefile.inc"
