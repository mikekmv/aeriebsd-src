
PROG=	systrace
CFLAGS+=-I. -I${.CURDIR} -I${.CURDIR}/../../sys
CFLAGS+=-Wall -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith \
	-Wno-uninitialized
CFLAGS+=-DYY_NO_UNPUT
DPADD+=	${LIBEVENT}
LDADD+=	-levent

SRCS=	cradle.c filter.c intercept-translate.c intercept.c \
	linux-translate.c openbsd-syscalls.c util.c \
	policy.c systrace-errno.h systrace-error.c \
	systrace-translate.c systrace.c alias.c register.c \
	parse.y lex.l

.include <bsd.prog.mk>
