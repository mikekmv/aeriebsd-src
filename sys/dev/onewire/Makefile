
AWK?=	awk

onewiredevs.h onewiredevs_data.h: onewiredevs
	/bin/rm -f onewiredevs.h onewiredevs_data.h
	${AWK} -f onewiredevs onewiredevs
