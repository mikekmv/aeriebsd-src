
PROG=	fcom_${TARGMACH}
#MAN=	fcom.1
NOMAN=
BINDIR=	/usr/libexec

TARGOS=	aeriebsd
CLEANFILES=gram.c gram.h external.h external.c mkext tokdefs

CFLAGS+=-g -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror
CPPFLAGS+=-DPCC_DEBUG -DGCC_COMPAT -DTARGOS=${TARGOS} -Dos_${TARGOS}
CPPFLAGS+=-DFCOM -DLANG_F77
CPPFLAGS+=-I. -I${.CURDIR}/.. -I${.CURDIR} -I${.CURDIR}/../../mip
CPPFLAGS+=-I${.CURDIR}/../../${TARGMACH}
CPPFLAGS+=-I${.CURDIR}/../..
#LFLAGS+=-CFe

.PATH:	${.CURDIR}/../../${TARGMACH}
.PATH:	${.CURDIR}/../../mip
.PATH:	${.CURDIR}/..

SRCS=	main.c init.c proc.c gram.c lex.c equiv.c data.c expr.c exec.c \
	intr.c io.c misc.c error.c put.c putscj.c flocal.c match.c \
	reader.c optim2.c regs.c local2.c order.c table.c common.c external.c

tokdefs: tokens
	@rm -f $@
	grep -n . < ${.CURDIR}/../tokens | \
	    sed "s/\([^:]*\):\(.*\)/#define \2 \1/" > $@

gram.c: gram.head gram.dcl gram.expr gram.exec gram.io tokdefs
	@rm -f $@
	(sed "s/#define/%token/" < tokdefs; \
	    cat ${.ALLSRC:Ntokdefs} ) > gram.in
	$(YACC) $(YFLAGS) gram.in
	@mv y.tab.c gram.c
	@mv y.tab.h gram.h
	@rm -f gram.in

mkext: mkext.c table.c common.c
	$(CC) $(LDSTATIC) $(CPPFLAGS) $(CFLAGS) -DMKEXT -o mkext \
	    ${.CURDIR}/../../mip/mkext.c ${.CURDIR}/../../${TARGMACH}/table.c \
	    ${.CURDIR}/../../mip/common.c

external.h external.c: mkext
	./mkext

.include "../../../Makefile.inc"
