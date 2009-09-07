#
# Makefile for the cc part of pcc.
#
PROG=   	cc
TARGOS=		aeriebsd
TARGMACH=       ${MACHINE_ARCH}

CPPFLAGS+=	-I${.CURDIR}/..
CPPFLAGS+=	-DDEFMACH=\"${TARGMACH}\"

.include <bsd.prog.mk>
.include "../../Makefile.inc"
