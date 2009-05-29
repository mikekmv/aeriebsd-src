
AWK=	awk

eisadevs.h eisadevs_data.h: eisadevs
	/bin/rm -f eisadevs.h eisadevs_data.h
	${AWK} -f eisadevs eisadevs
