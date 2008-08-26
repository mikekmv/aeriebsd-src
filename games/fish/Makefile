
PROG=	fish
MAN=	fish.6

beforeinstall: 
	${INSTALL} ${INSTALL_COPY} -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${.CURDIR}/fish.instr ${DESTDIR}/usr/share/games/fish.instr

.include <bsd.prog.mk>
