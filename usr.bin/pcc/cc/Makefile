#
# Makefile for the cc part of pcc.
#
PROG=   	cc
PREFIX=		/usr/local
BINDIR=		${PREFIX}/bin
MANDIR=		${PREFIX}/man/man
TARGOS=		aeriebsd
TARGMACH=       ${MACHINE_ARCH}

CFLAGS+=	-DLIBEXECDIR=\"${PREFIX}/libexec/\"
CPPFLAGS+=	-I${.CURDIR}/..
CPPFLAGS+=	-DDEFMACH=\"${TARGMACH}\"

.include <bsd.prog.mk>
