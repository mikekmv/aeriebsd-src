
#
# Be sure to change src/distrib/special/kbd/Makefile as well if you add or
# remove architectures on this list!
#
    
PROG=	kbd

SRCS=	main.c kbd_wscons.c
LDADD=	-lkvm
DPADD=	${LIBKVM}

MAN=		kbd.8
MANSUBDIR=	alpha amd64 hp300 hppa i386 landisk mac68k macppc \
		sgi sparc sparc64 vax zaurus

.include <bsd.prog.mk>
