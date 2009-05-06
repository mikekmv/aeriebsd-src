
PROG=	dc
SRCS=	dc.c bcode.c inout.c mem.c stack.c
COPTS+= -Wall
LDADD=	-lcrypto
DPADD=	${LIBCRYPTO}

# This just gets installed verbatim
.if make(install)
SUBDIR+= USD.doc
.endif

.include <bsd.prog.mk>
