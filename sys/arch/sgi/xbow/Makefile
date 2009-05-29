
AWK=	awk

xbowdevs.h xbowdevs_data.h: xbowdevs
	/bin/rm -f xbowdevs.h xbowdevs_data.h
	${AWK} -f xbowdevs xbowdevs
