.ifdef DESTDIR
snap_md: bsd bootblocks distrib
	cp ${.CURDIR}/../sys/arch/mac68k/compile/GENERIC/bsd \
	    ${DESTDIR}/snapshot/bsd
	printf "disable ncrscsi\nenable sbc\nquit" | config -e \
	    -o ${DESTDIR}/snapshot/bsdsbc ${DESTDIR}/snapshot/bsd

bsd:
	cd ${.CURDIR}/../sys/arch/mac68k/conf && config GENERIC
	cd ${.CURDIR}/../sys/arch/mac68k/compile/GENERIC && \
	    ${MAKE} clean && ${MAKE} depend && exec ${MAKE}

bootblocks:
	#none needed

.PHONY: bsd bootblocks

.endif	# DESTDIR check
