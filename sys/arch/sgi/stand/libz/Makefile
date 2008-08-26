
S=${.CURDIR}/../../../..
ZDST=${.OBJDIR}

.PATH: ${S}/lib/libz

.include "${S}/lib/libz/Makefile"
CFLAGS+= ${SAABI} -mno-abicalls -D_NO_ABICALLS ${CEXTRAFLAGS} ${AFLAGS} -I${S}
