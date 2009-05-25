
.include <bsd.own.mk>

NOMAN=

PROG=cpp
SRCS=
INSTALL_STRIP=

cpp: cpp.sh
	sed -e 's/@dollaropt@//' ${.CURDIR}/cpp.sh >$@

.include <bsd.prog.mk>
