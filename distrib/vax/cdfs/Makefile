
TOP=    ${.CURDIR}/..

.include "${TOP}/Makefile.inc"

FLOPPY=	floppy${REV}.fs
CDROM=	cd${REV}.iso

# The floppy image works fine as a CD image, too

install:
	cp ${DESTDIR}/snapshot/${FLOPPY} ${DESTDIR}/snapshot/${CDROM}

.include <bsd.obj.mk>
.include <bsd.subdir.mk>
