
.if ${MACHINE} == "sparc64"

PROG=mksuncd
BINDIR=/usr/bin

CFLAGS+=-Werror -Wall -Wstrict-prototypes -Wmissing-prototypes

.else
NOPROG=
.endif

.include <bsd.prog.mk>

