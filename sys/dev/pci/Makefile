
AWK=	awk

pcidevs.h pcidevs_data.h: pcidevs devlist2h.awk
	/bin/rm -f pcidevs.h pcidevs_data.h
	${AWK} -f devlist2h.awk pcidevs
