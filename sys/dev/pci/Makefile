
AWK=	awk

pcidevs.h pcidevs_data.h: pcidevs
	/bin/rm -f pcidevs.h pcidevs_data.h
	${AWK} -f pcidevs pcidevs
