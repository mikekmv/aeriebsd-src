
AWK=	awk
PROG=	hildevs
SOURCE=	hildevs

${SOURCE}_data.h: ${SOURCE} ${PROG}
	/bin/rm -f $@
	${AWK} -f ${PROG} ${SOURCE} > $@
