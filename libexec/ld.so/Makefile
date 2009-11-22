
SUBDIR=ldconfig ldd

MAN=	ld.so.1

.include <bsd.own.mk>
.if defined(NOPIC)
NOPROG=
.else
PROG=	ld.so
.endif

VPATH=${.CURDIR}/../../lib/libc/string

SRCS=	ldasm.S loader.c resolve.c dlfcn.c dl_printf.c rtld_machine.c
SRCS+=	util.c sod.c dir.c library_subr.c dl_prebind.c
.if (${MACHINE_ARCH} == "i386")
SRCS+=	library_mquery.c
.else
SRCS+=	library.c
.endif
# library substitutes
SRCS+=	strsep.c strtol.c

.include "${.CURDIR}/${MACHINE_ARCH}/Makefile.inc"
.PATH:	${.CURDIR}/${MACHINE_ARCH}

CFLAGS += -Wall -Werror
CFLAGS += -I${.CURDIR} -I${.CURDIR}/${MACHINE_ARCH} \
	-Dstrsep=_dl_strsep
INSTALL_STRIP=

ELF_LDFLAGS+=--shared -Bsymbolic --no-undefined

$(PROG):
	$(LD) -x -e _dl_start $(ELF_LDFLAGS) -o $(PROG) $(OBJS) $(LDADD)

.include <bsd.prog.mk>
