
DOCDIR=	/usr/share/doc/psd
FILES=	00.contents Makefile Title
SUBDIR=	01.cacm 02.implement 03.iosys 04.uprog 05.sysman 06.Clang \
	20.ipctut 21.ipc

Title.ps: ${FILES}
	${ROFF} Title > ${.TARGET}
Title.txt: ${FILES}
	${ROFF} -Tascii Title > ${.TARGET}

contents.ps: ${FILES}
	${ROFF} -ms 00.contents > ${.TARGET}
contents.txt: ${FILES}
	${ROFF} -Tascii -ms 00.contents > ${.TARGET}

beforeinstall:
	install -c -o ${DOCOWN} -g ${DOCGRP} -m ${DOCMODE} ${FILES} \
	    ${DESTDIR}${DOCDIR}

.include <bsd.subdir.mk>
