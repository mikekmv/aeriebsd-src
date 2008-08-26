
AWK=	awk
UNAME=	uname

PROG=	makemap.awk
MAP=	../pckbc/wskbdmap_mfii.c

all:	usbdevs.h ukbdmap.c

usbdevs.h usbdevs_data.h: usbdevs devlist2h.awk
	/bin/rm -f usbdevs.h usbdevs_data.h
	${AWK} -f devlist2h.awk usbdevs

ukbdmap.c: ${MAP} ${PROG}
	/bin/rm -f $@
	${AWK} -f ${PROG} ${MAP} > $@
