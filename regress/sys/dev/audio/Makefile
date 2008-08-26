
PROG=autest
SRCS=autest.c adpcm.c law.c
CFLAGS+=-Wall -Wstrict-prototypes -Wmissing-prototypes
MAN1=autest.1
LDADD=-lm

.ifndef DO_AUTEST
REGRESS_SKIP=
.endif

.include <bsd.regress.mk>
