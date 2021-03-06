
.ifndef __INCLUDED_STAND_MAKEFILE_INC
__INCLUDED_STAND_MAKEFILE_INC=

#CEXTRAFLAGS?=-D_LP64

BINDIR=		/usr/mdec

STANDALONE?=	-D_STANDALONE

.if ${MACHINE} == "sgi"
CPPFLAGS+=	${STANDALONE}
CPPFLAGS+=	-I.

CFLAGS+=	-fno-stack-protector
CFLAGS+=	-fno-builtin-vprintf -fno-builtin-printf -fno-builtin-putchar
SAABI=		-mabi=32 -mips2 -Wl,-m=elf32btsmip
AS=		as -32
LD=		ld -m elf32btsmip
.endif

### Figure out what to use for libsa
LIBSADIR=               ${.CURDIR}/../libsa

.if exists(${LIBSADIR}/obj.${MACHINE})
LIBSAOBJDIR=            ${LIBSADIR}/obj.${MACHINE}
.elif exists(${LIBSADIR}/obj)
LIBSAOBJDIR=            ${LIBSADIR}/obj
.else
LIBSAOBJDIR=            ${LIBSADIR}
.endif

LIBSA=                  ${LIBSAOBJDIR}/libsa.a

### Figure out what to use for libz
LIBZDIR=                ${.CURDIR}/../libz

.if exists(${LIBZDIR}/obj.${MACHINE})
LIBZOBJDIR=             ${LIBZDIR}/obj.${MACHINE}
.elif exists(${LIBZDIR}/obj)
LIBZOBJDIR=             ${LIBZDIR}/obj
.else
LIBZOBJDIR=             ${LIBZDIR}
.endif

LIBZ=                   ${LIBZOBJDIR}/libz.a

.endif
