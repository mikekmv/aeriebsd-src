
LIB=	elf
SRCS=	elf32.c elf64.c
CLEANFILES+=elf32.c elf64.c
WANTLINT=

elf32.c: ${.CURDIR}/elf.c
	echo '#define ELFSIZE 32' | cat - $> > ${.TARGET}

elf64.c: ${.CURDIR}/elf.c
	echo '#define ELFSIZE 64' | cat - $> > ${.TARGET}

includes:
	@cmp -s ${DESTDIR}/usr/include/elfuncs.h ${.CURDIR}/elfuncs.h || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) \
	    ${.CURDIR}/elfuncs.h ${DESTDIR}/usr/include/elfuncs.h

.include <bsd.lib.mk>
