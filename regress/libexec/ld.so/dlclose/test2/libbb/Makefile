
LIB=		bb
SRCS=		bb.c
LDADD+=		-Wl,-E
LDADD+=		-Wl,-rpath,$(ZZ_OBJDIR)
LDADD+=		-L$(ZZ_OBJDIR) -lzz

regress: all

.include <bsd.lib.mk>
