
LIB= 	test
SRCS=	nover.c

regress: all

.include <bsd.lib.mk>

all:	 lib${LIB}.so

lib${LIB}.so: ${SOBJS} ${DPADD}
	@echo building shared ${LIB} library
	@rm -f lib${LIB}.so
	${CC} -shared ${PICFLAG} \
	    -o lib${LIB}.so `${LORDER} ${SOBJS}|tsort -q` ${LDADD}
