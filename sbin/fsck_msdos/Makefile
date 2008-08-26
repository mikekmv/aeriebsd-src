
PROG=	fsck_msdos
MAN=	fsck_msdos.8
SRCS=	main.c check.c boot.c fat.c dir.c fsutil.c
CFLAGS+= -I${.CURDIR}/../fsck
.PATH: ${.CURDIR}/../fsck

.include <bsd.prog.mk>
