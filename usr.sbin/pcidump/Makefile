
.if (${MACHINE} == "hp300") || (${MACHINE} == "mac68k") || \
    (${MACHINE} == "sparc") || (${MACHINE} == "vax")
NOPROG=yes
.else
PROG=	pcidump
.endif

MAN=	pcidump.8

.include <bsd.prog.mk>
