
AFSTOPDIR	= ../..
.include <../../Makefile.inc>

.PATH:	$(ROKENSRC)

PROG	= pts
SRCS	= pts.c \
	  estrdup.c
LDADD	+= ${LIBARLA} -lkrb5 -lcrypto -lasn1 -lcurses
DPADD	+= ${LIBARLA} ${LIBKRB5} ${LIBCRYPTO} ${LIBASN1} ${LIBCURSES}

.PATH: ${AFSSRCDIR}/appl/pts

SRCS	+= arlalib.c fs_lib.c
CFLAGS	+= -I${AFSSRC}/appl/lib
.PATH: ${AFSSRC}/appl/lib
.PATH: ${AFSSRC}/appl/pts
.PATH: ${AFSSRC}/arlad

.include <bsd.prog.mk>
