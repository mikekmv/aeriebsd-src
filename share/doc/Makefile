#	@(#)Makefile	8.1 (Berkeley) 6/5/93

FILES=	README
SUBDIR=	psd smm usd

all depend includes lint tags:

realinstall:
	install -c -o ${DOCOWN} -g ${DOCGRP} -m ${DOCMODE} ${FILES} \
	    ${DESTDIR}${DOCDIR}

.include <bsd.subdir.mk>
