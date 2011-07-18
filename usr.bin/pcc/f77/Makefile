
PROG=	f77
TARGOS=	aeriebsd
TARGMACH=${MACHINE_ARCH}

CPPFLAGS+=-DLANG_F77 -I${.CURDIR}/.. -DDEFMACH=\"${TARGMACH}\"
CPPFLAGS+=-DLIBEXECDIR=\"/usr/libexec\" -DLIBDIR=\"/usr/lib\"

.include <bsd.prog.mk>
.include "../../Makefile.inc"
