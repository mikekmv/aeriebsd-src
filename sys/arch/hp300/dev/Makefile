
AWK=	awk

diodevs.h diodevs_data.h: diodevs
	/bin/rm -f diodevs.h diodevs_data.h
	${AWK} -f diodevs diodevs
