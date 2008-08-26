
PROG=	ifconfig
MAN=	
COPTS+=-Os -DSMALL
.PATH:  ${.CURDIR}/../../../sbin/ifconfig

CPPFLAGS+=-DINET6

# kame scopeid hack
CPPFLAGS+=-DKAME_SCOPEID

.include <bsd.prog.mk>
