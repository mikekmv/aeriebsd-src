
AWK=	awk

eisadevs.h eisadevs_data.h: eisadevs devlist2h.awk
	/bin/rm -f eisadevs.h eisadevs_data.h
	${AWK} -f devlist2h.awk eisadevs
