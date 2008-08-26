
PROG=	top

CFLAGS+=-I. -Wall -g -Wmissing-prototypes
SRCS=	commands.c display.c machine.c screen.c top.c username.c utils.c
DPADD=	${LIBCURSES}
LDADD=	-lcurses

.include <bsd.prog.mk>

