#
# This Makefile includes boiler-plate stuff included by each subdir's Makefile.

OBJS+= ${ASM}
POBJS+=	${ASM:.o=.po}
CLEANFILES+= ${ASM} ${POBJS}

LIB=    ${MACHINE_ARCH}
