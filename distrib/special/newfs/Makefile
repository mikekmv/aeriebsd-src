
PROG=	newfs
MAN=
SRCS=	dkcksum.c getmntopts.c newfs.c mkfs.c
COPTS+=-Os
CFLAGS+= -I${.CURDIR}/../../../sbin/mount
.PATH:  ${.CURDIR}/../../../sbin/newfs
.PATH:  ${.CURDIR}/../../../sbin/mount
.PATH:	${.CURDIR}/../../../sbin/disklabel

LDADD+=	-lutil
DPADD+=	${LIBUTIL}

.include <bsd.prog.mk>
