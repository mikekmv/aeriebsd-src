.ifdef DESTDIR
snap_md: bsd bsd.mp bootblocks distrib
	cp ${.CURDIR}/../sys/arch/sparc64/compile/GENERIC/bsd \
	    ${DESTDIR}/snapshot/bsd
	cp ${.CURDIR}/../sys/arch/sparc64/compile/GENERIC.MP/bsd \
	    ${DESTDIR}/snapshot/bsd.mp

bsd:
	cd ${.CURDIR}/../sys/arch/sparc64/conf && config GENERIC
	cd ${.CURDIR}/../sys/arch/sparc64/compile/GENERIC && \
	    ${MAKE} clean && ${MAKE} depend && exec ${MAKE}

bsd.mp:
	cd ${.CURDIR}/../sys/arch/sparc64/conf && config GENERIC.MP
	cd ${.CURDIR}/../sys/arch/sparc64/compile/GENERIC.MP && \
	    ${MAKE} clean && ${MAKE} depend && exec ${MAKE}

bootblocks:
	cp ${DESTDIR}/usr/mdec/*boot* ${DESTDIR}/snapshot

.PHONY: bsd bsd.mp bootblocks

.endif	# DESTDIR check
