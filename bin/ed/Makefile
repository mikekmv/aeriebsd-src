
PROG=	ed
CFLAGS+=-DBACKWARDS -DDES
SRCS=	 buf.c cbc.c glbl.c io.c main.c re.c sub.c undo.c

#LINKS=  ${BINDIR}/ed ${BINDIR}/red
#MLINKS= ed.1 red.1

# These just get installed verbatim
.if make(install)
SUBDIR+= USD.doc/09.edtut USD.doc/10.edadv
.endif

.include <bsd.prog.mk>
