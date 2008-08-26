.if ${MACHINE_ARCH} == "m68k"

AS?=	as
AS+=	-m68060
LD=	ld

.SUFFIXES:	.o .S .sa .defs .h

.sa.S:
	sh ${.CURDIR}/asm2gas ${.IMPSRC} >${.TARGET}

060sp.o: netbsd.S inetbsd.S fnetbsd.S isp.S fpsp.S copyright.S
	$(AS) -I${.CURDIR} -o ${.TARGET} ${.CURDIR}/netbsd.S

clean cleandir:
	rm -f 060sp.o isp.S fpsp.S

_SUBDIRUSE:

.else
clean cleandir:

.endif

all:

depend:

install:

includes:

.include <bsd.obj.mk>
.include <bsd.own.mk>
