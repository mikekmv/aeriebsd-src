
LIB=	ossaudio
MAN=	ossaudio.3

SRCS=	ossaudio.c

CPPFLAGS+= -I${.CURDIR}

includes:
	@cd ${.CURDIR}; cmp -s soundcard.h ${DESTDIR}/usr/include/soundcard.h || \
	  ${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) soundcard.h \
	  ${DESTDIR}/usr/include

.include <bsd.lib.mk>

