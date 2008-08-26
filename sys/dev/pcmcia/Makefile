
AWK=	awk

pcmciadevs.h: pcmciadevs devlist2h.awk
	/bin/rm -f pcmciadevs.h
	${AWK} -f devlist2h.awk pcmciadevs
