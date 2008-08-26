
.include <bsd.own.mk>

NOMAN=

PROG=cpp
SRCS=
INSTALL_STRIP=

cpp: cpp.sh
.if ${USE_GCC3:L} == "no"
	sed -e 's/@dollaropt@/-$$/' ${.CURDIR}/cpp.sh >$@
.else
	sed -e 's/@dollaropt@//' ${.CURDIR}/cpp.sh >$@
.endif

.include <bsd.prog.mk>
