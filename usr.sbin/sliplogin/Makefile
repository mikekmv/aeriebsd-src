
PROG=	sliplogin
MAN=	sliplogin.8
FILES=	slip.hosts slip.login
BINOWN=	root
BINGRP=	network
BINMODE=4550

.include <bsd.prog.mk>

afterinstall:
	@echo installing ${FILES}
	@-for i in ${FILES}; do \
		if [ ! -f ${DESTDIR}/etc/sliphome/$$i ]; then \
			${INSTALL} ${INSTALL_COPY} -o root -g wheel ${.CURDIR}/$$i \
				${DESTDIR}/etc/sliphome/$$i; \
		fi; \
	done
