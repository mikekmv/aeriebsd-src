
CFLAGS += -fpic -msoft-float -I${LIBCSRCDIR}/arch/sparc
AFLAGS = ${CFLAGS} -DSTRONG_SPARC
LIBCSRCDIR=${.CURDIR}/../../lib/libc
.include "${LIBCSRCDIR}/arch/sparc/Makefile.inc"
.PATH: ${LIBCSRCDIR}/arch/sparc/gen/
SRCS+=umul.S mul.S
