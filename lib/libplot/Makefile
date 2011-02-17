
LIB=	plot
CFLAGS+=-Wall -I${.CURDIR}
SRCS=	plot_open.c plot_close.c plot_erase.c plot_space.c \
	plot_move.c plot_point.c plot_dot.c plot_cont.c plot_line.c \
	plot_circle.c plot_arc.c plot_label.c plot_linemod.c \
	plotsubr.c
MAN=	plot.3 plotcap.5
MLINKS+=plot.3 plot_arc.3 plot.3 plot_circle.3 plot.3 plot_close.3 \
	plot.3 plot_cont.3 plot.3 plot_erase.3 plot.3 plot_label.3 \
	plot.3 plot_line.3 plot.3 plot_linemod.3 plot.3 plot_move.3 \
	plot.3 plot_open.3 plot.3 plot_point.3 plot.3 plot_space.3
CLEANFILES+=	plotcap.db

all: plotcap.db

afterinstall:
	${INSTALL} ${INSTALL_COPY} -m 0444 -o $(BINOWN) -g $(BINGRP) \
	    ${.CURDIR}/plotcap ${DESTDIR}/usr/share/misc
	${INSTALL} ${INSTALL_COPY} -m 0444 -o $(BINOWN) -g $(BINGRP) \
	    plotcap.db ${DESTDIR}/usr/share/misc

includes:
	@cmp -s ${DESTDIR}/usr/include/plot.h ${.CURDIR}/plot.h || \
	${INSTALL} ${INSTALL_COPY} -m 444 -o $(BINOWN) -g $(BINGRP) \
	    ${.CURDIR}/plot.h ${DESTDIR}/usr/include/plot.h

plotcap.db: ${.CURDIR}/plotcap
	cap_mkdb -f plotcap ${.CURDIR}/plotcap

.include <bsd.lib.mk>
