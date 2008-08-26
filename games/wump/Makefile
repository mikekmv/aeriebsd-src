
PROG=	wump
MAN=	wump.6

beforeinstall:
	${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${.CURDIR}/wump.info ${DESTDIR}/usr/share/games

.include <bsd.prog.mk>
