
# Makefile for alpha tags file and boot blocks

S=${.CURDIR}/../..
.include "$S/kern/Make.tags.inc"

NOPROG=	noprog
NOMAN=	noman

SUBDIR=	stand

TALPHA=	$S/arch/alpha/tags
SALPHA=	$S/arch/alpha/alpha/*.[ch] $S/arch/alpha/include/*.h \
	$S/arch/alpha/tc/*.[ch] $S/arch/alpha/dev/*.[ch] \
	$S/arch/alpha/eisa/*.[ch] $S/arch/alpha/isa/*.[ch] \
	$S/arch/alpha/pci/*.[ch]
AALPHA=	$S/arch/alpha/alpha/*.[sS] $S/arch/alpha/pci/*.[Ss] \
	$S/lib/libkern/arch/alpha/*.[sS]

# Directories in which to place alpha tags links
DALPHA=	alpha eisa isa dev pci tc include

tags:
	-ctags -wdtf ${TALPHA} ${COMM} ${SALPHA}
	egrep "^ENTRY(.*)|^ALTENTRY(.*)" ${AALPHA} | \
	    sed "s;\([^:]*\):\([^(]*\)(\([^, )]*\)\(.*\);\3	\1	/^\2(\3\4$$/;" \
		>> ${TALPHA}
	sort -o ${TALPHA} ${TALPHA}

links:
	-for i in ${DALPHA}; do \
	    (cd $$i && { rm -f tags; ln -s ${TALPHA} tags; }) done

obj: _SUBDIRUSE

.include <bsd.prog.mk>
