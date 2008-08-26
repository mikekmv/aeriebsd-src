
AWK=	awk

miidevs.h: miidevs devlist2h.awk
	/bin/rm -f miidevs.h
	${AWK} -f devlist2h.awk miidevs
