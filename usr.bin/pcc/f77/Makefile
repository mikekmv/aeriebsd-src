#
# Makefile for the f77 part of pcc.
#
PROG=   	f77
PREFIX=		/usr/local
BINDIR=		${PREFIX}/bin
MANDIR=		${PREFIX}/man/man
TARGOS=		aeriebsd
TARGMACH=       ${MACHINE_ARCH}

CPPFLAGS+=	-DLIBEXECDIR=\"${PREFIX}/libexec\"
CPPFLAGS+=	-DLIBDIR=\"${PREFIX}/lib\"
CPPFLAGS+=	-I${.CURDIR}/..
CPPFLAGS+=	-DDEFMACH=\"${TARGMACH}\"

.include <bsd.prog.mk>
