
.PATH:		${.CURDIR}/..

LIB=	noexec
NOMAN=	yes
SRCS=	sudo_noexec.c
LIBDIR=	/usr/libexec

DEBUGLIBS= no
NOPROFILE= yes

install:
	${INSTALL} ${INSTALL_COPY} -o ${LIBOWN} -g ${LIBGRP} -m ${LIBMODE} \
	    lib${LIB}.so.${SHLIB_MAJOR}.${SHLIB_MINOR} \
	    ${DESTDIR}${LIBDIR}/sudo_noexec.so

.include <bsd.lib.mk>
