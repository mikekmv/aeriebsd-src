
PROG=	mount_xfs
SRCS=	mount_xfs.c getmntopts.c
MAN=	mount_xfs.8

MOUNT=	${.CURDIR}/../mount
CFLAGS+= -I${.CURDIR}/../../sys -I${MOUNT}
.PATH:	${MOUNT}

.include <bsd.prog.mk>
