
PROG=   cpp
TARGOS=	aeriebsd
BINDIR=	/usr/libexec

CFLAGS+=-DCPP_DEBUG -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror
CPPFLAGS+=-DGCC_COMPAT -I. -I${.CURDIR} -I${.CURDIR}/..

SRCS=cpy.y token.c cpp.c

.include <bsd.prog.mk>
.include "../../Makefile.inc"
