
.include <bsd.own.mk>

TARGET_MACHINE_ARCH?=	${MACHINE_ARCH}

PROG=	gprof
SRCS=	gprof.c arcs.c dfn.c lookup.c ${TARGET_MACHINE_ARCH}.c hertz.c \
	printgprof.c printlist.c
CFLAGS+= -I. -DMD_INCLUDE=\"${TARGET_MACHINE_ARCH}.h\"

# XXX - need support for multiple archs.
.if (${ELF_TOOLCHAIN:L} == "yes")
SRCS+=elf.c
.else
SRCS+=aout.c
.endif

beforeinstall:
	${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${.CURDIR}/gprof.flat ${.CURDIR}/gprof.callg \
	    ${DESTDIR}/usr/share/misc

.if make(install)
SUBDIR+= PSD.doc
.endif

.include <bsd.prog.mk>
