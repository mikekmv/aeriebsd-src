
LIB=	elf
SRCS=	elf32.c elf64.c
CFLAGS+=-Wall
CLEANFILES+=elf32.c elf64.c
WANTLINT=
MAN=	elf_size.3
MLINKS=	elf_size.3 elf_fix_header.3 elf_size.3 elf_fix_note.3 \
	elf_size.3 elf_load_shdrs.3 elf_size.3 elf_save_shdrs.3 \
	elf_size.3 elf_fix_shdrs.3 elf_size.3 elf_shstrload.3 \
	elf_size.3 elf_load_phdrs.3 elf_size.3 elf_fix_phdrs.3 \
	elf_size.3 elf_strload.3 elf_size.3 elf_symload.3 \
	elf_size.3 elf_fix_sym.3 elf_size.3 elf2nlist.3

elf32.c: ${.CURDIR}/elf.c
	echo '#define ELFSIZE 32' | cat - $> > ${.TARGET}

elf64.c: ${.CURDIR}/elf.c
	echo '#define ELFSIZE 64' | cat - $> > ${.TARGET}

includes:
	@cmp -s ${DESTDIR}/usr/include/elfuncs.h ${.CURDIR}/elfuncs.h || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) \
	    ${.CURDIR}/elfuncs.h ${DESTDIR}/usr/include/elfuncs.h

.include <bsd.lib.mk>
