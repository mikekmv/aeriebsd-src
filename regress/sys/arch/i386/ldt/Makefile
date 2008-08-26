#
# C Optimizer (-O) breaks this program - don't use !!
CFLAGS=-g
PROG=	testldt

DPADD+=	${LIBARCH}
LDADD+=	-li386

USERLDT_ENABLE!=sysctl -n machdep.userldt

.ifmake !obj && !clean && !cleandir && !depend && !regress
.INTERRUPT:
	-@${SUDO} sysctl -q machdep.userldt=${USERLDT_ENABLE}

.END:
	-@${SUDO} sysctl -q machdep.userldt=${USERLDT_ENABLE}
.endif

.if ${MACHINE_ARCH} != "i386"
REGRESS_SKIP="yes"
.endif

do-testldt:	${PROG}
	@${SUDO} sysctl -q machdep.userldt=1
	./${PROG}

REGRESS_TARGETS=do-testldt
REGRESS_ROOT_TARGETS=${REGRESS_TARGETS}
.PHONY: ${REGRESS_TARGETS}

.include <bsd.regress.mk>

