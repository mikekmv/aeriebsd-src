
MANSUBDIR=amd64
MAN+=	amd64_iopl.2 amd64_get_ioperm.2
MLINKS+=amd64_get_ioperm.2 amd64_set_ioperm.2

.if ${MACHINE_ARCH} == "amd64"
.PATH: ${LIBC}/amd64
NOPIC=
SRCS+=	amd64_iopl.c amd64_get_ioperm.c amd64_set_ioperm.c
.include <bsd.lib.mk>
.else
NOPROG=
.include <bsd.prog.mk>
.endif

