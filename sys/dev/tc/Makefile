
AWK=	awk

tcdevs_data.h: tcdevs devlist2h.awk
	/bin/rm -f tcdevs_data.h
	${AWK} -f devlist2h.awk tcdevs
