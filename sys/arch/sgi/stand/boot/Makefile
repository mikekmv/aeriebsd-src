
PROG=		boot
NOMAN=		noman

CFLAGS+=	-O2 ${SAABI} -mno-abicalls -D_NO_ABICALLS -nostdinc -D__sgi__ \
		-I${.CURDIR}/../include -I${.CURDIR}/../../../.. \
		-I${.CURDIR}/../../../../lib/libsa \
		-I${.OBJDIR}
CFLAGS+=	-D__INTERNAL_LIBSA_CREAD ${STANDALONE}

AFLAGS+=	${SAABI}

LDFLAGS+=	${SALDFLAGS} -T ${.CURDIR}/ld.script -e __start

S=		${.CURDIR}/../../../..
SRCS=		start.S boot.c filesystem.c conf.c diskio.c arcbios.c strstr.c

.PATH:		${S}/lib/libsa
SRCS+=		loadfile.c

.PATH:		${S}/lib/libkern/arch/mips64 ${S}/lib/libkern
SRCS+=		strlcpy.c memcpy.c strlen.c strrchr.c strlcat.c ashrdi3.c strncmp.c \
		strcmp.S udivdi3.c umoddi3.c qdivrem.c

CLEANFILES+=	machine mips64

.if !make(libdep) && !make(sadep) && !make(salibdir) && !make(obj)
.BEGIN:
	@([ -h machine ] || ln -s ${.CURDIR}/../../include machine)
	@([ -h mips64 ] || ln -s ${.CURDIR}/../../../mips64/include mips64)
.endif

${PROG}: $(OBJS) $(LDADD)
	$(LD) $(LDFLAGS) -o boot $(OBJS) -L${LIBSADIR} ${LIBSA} \
	-L${LIBZDIR} ${LIBZ}

install:
	install -c -m 555 -g bin -o bin ${PROG} ${DESTDIR}/usr/mdec

.include <bsd.prog.mk>
