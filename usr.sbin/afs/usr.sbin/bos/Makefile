
AFSTOPDIR	= ../..
.include <../../Makefile.inc>

.PATH:	$(ROKENSRC)

PROG	= bos
BINDIR	= /usr/sbin
MAN	= bos.8
LDADD	+= ${LIBARLA} -lkrb5 -lcrypto -lasn1 -lcurses
DPADD	+= ${LIBARLA} ${LIBKRB5} ${LIBCRYPTO} ${LIBASN1} ${LIBCURSES}

CFLAGS	+= -I${AFSSRC}/appl/lib

SRCS	= bos.c \
	  bos_addhost.c \
	  bos_adduser.c \
	  bos_getrestart.c \
	  bos_listhosts.c \
	  bos_listusers.c \
	  bos_removehost.c \
	  bos_restart.c \
	  bos_start.c \
	  bos_status.c \
	  bos_stop.c \
	  estrdup.c \
	  emalloc.c \
	  erealloc.c

.PATH: ${AFSSRC}/appl/bos

#SRCS	+= arladebu.c fs_lib.c

.PATH: ${AFSSRC}/appl/lib
.PATH: ${AFSSRC}/arlad

.include <bsd.prog.mk>
