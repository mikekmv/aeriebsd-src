PROG=	mount_milfs
SRCS=	mount_milfs.c getmntopts.c
#MAN=	mount_milfs.8
NOMAN=

MOUNT=	${.CURDIR}/../mount
CFLAGS+= -I${MOUNT}
.PATH:	${MOUNT}

.include <bsd.prog.mk>
