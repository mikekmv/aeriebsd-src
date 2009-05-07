
.PATH:		${.CURDIR}/..

PROG=	visudo
SRCS=	pwutil.c visudo.c
BINDIR=	/usr/sbin
CPPFLAGS+=	-I..

POD2MAN=/usr/bin/pod2man
MAN=	visudo.8
VERSION!=uname -rs
MANDATE!=date '+%B %e, %Y'

CLEANFILES+= ${MAN}

visudo.8: visudo.pod
	sed -f ${.CURDIR}/../varsub ${.ALLSRC} | ${POD2MAN} --quotes=none \
	    --name="VISUDO" --section=8 --release="${VERSION}" \
	    --date="${MANDATE}" --center="AerieBSD Reference Manual" | \
	    sed '1s/^/.if n .ll 78n /' > $@

afterdepend: ${MAN}

.include <bsd.prog.mk>
