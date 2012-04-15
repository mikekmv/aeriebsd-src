
PROG=	cxxcom_${TARGMACH}
MAN=	cxxcom.1
BINDIR=	/usr/libexec

TARGOS=	aeriebsd
CLEANFILES=cgram.h external.h external.c mkext

CPPFLAGS+=-DPCC_DEBUG -DGCC_COMPAT -DTARGOS=${TARGOS} -Dos_${TARGOS} -DLANG_CXX
CPPFLAGS+=-I. -I${.CURDIR}/.. -I${.CURDIR} -I${.CURDIR}/../../mip
CPPFLAGS+=-I${.CURDIR}/../../${TARGMACH}
CPPFLAGS+=-I${.CURDIR}/../..
CFLAGS+=-g -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror
#LFLAGS+=-CFe

.PATH:	${.CURDIR}/../../${TARGMACH}
.PATH:	${.CURDIR}/../../mip
.PATH:	${.CURDIR}/..

SRCS=	builtins.c optim.c pftn.c scan.l trees.c cgram.y inline.c symtabs.c \
	gcc_compat.c init.c local.c code.c stabs.c match.c reader.c optim2.c \
	regs.c local2.c order.c table.c common.c main.c external.c cxxcode.c

cgram.c: cgram.y
	$(YACC) $(YFLAGS) -d $<
	mv y.tab.c cgram.c
	mv y.tab.h cgram.h

mkext: mkext.c table.c common.c
	$(CC) $(LDSTATIC) $(CPPFLAGS) $(CFLAGS) -DMKEXT -o mkext \
	    ${.CURDIR}/../../mip/mkext.c ${.CURDIR}/../../${TARGMACH}/table.c \
	    ${.CURDIR}/../../mip/common.c

external.h external.c: mkext
	./mkext

pftn.o: cgram.c

cleandepend:
	rm -f .depend ${.CURDIR}/tags cgram.h

.include "../../../Makefile.inc"
