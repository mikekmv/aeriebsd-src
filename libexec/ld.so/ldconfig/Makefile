
PROG=	ldconfig
SRCS=	ldconfig.c shlib.c etc.c prebind_delete.c debug.c prebind.c library.c sod.c
LDDIR?= $(.CURDIR)/..
#CFLAGS+=-Werror
CFLAGS+=-I$(.CURDIR) -I$(.CURDIR)/..
LDSTATIC=${STATIC}
BINDIR=	/sbin
MAN=	ldconfig.8

#.PATH: $(LDDIR) $(LDDIR)/$(MACHINE_ARCH)

.include <bsd.prog.mk>
