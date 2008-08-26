
AWK=	awk

pnpdevs.h: pnpdevs devlist2h.awk
	/bin/rm -f pnpdevs.h
	${AWK} -f devlist2h.awk pnpdevs
