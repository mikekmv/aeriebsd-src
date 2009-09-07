#
# Makefile for the cpp part of pcc.
#
PROG=   	cpp
TARGOS=		aeriebsd
BINDIR=		/usr/libexec

CFLAGS+=	-DCPP_DEBUG -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror
CPPFLAGS+=	-I. -I${.CURDIR} -I${.CURDIR}/..

SRCS=cpy.y token.c cpp.c

.include <bsd.prog.mk>
.include "../../Makefile.inc"
