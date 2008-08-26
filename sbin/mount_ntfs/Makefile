
.if (${MACHINE} == "alpha" || ${MACHINE} == "amd64" || ${MACHINE} == "i386")
PROG=	mount_ntfs
SRCS=	mount_ntfs.c getmntopts.c
.else
NOPROG=
.endif

MAN=	mount_ntfs.8
MOUNT=	${.CURDIR}/../mount
CFLAGS+= -I${MOUNT}
.PATH:	${MOUNT}

.include <bsd.prog.mk>
