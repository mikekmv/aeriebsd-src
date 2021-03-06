#
#	NOTE: $S must correspond to the top of the 'sys' tree
SADIR?=	$S/lib/libsa

SADST?=	lib/sa
SAREL?=	../../
SALIB?=	${SADST}/libsa.a

SAMAKE= \
	cd ${SADST} && ${MAKE} -f ${SAREL}${SADIR}/Makefile \
	  CC="${CC}" XCFLAGS="${CFLAGS}" \
	  AS='${AS}' XAFLAGS='${AFLAGS}' \
	  LD='${LD}' \
	  XCPPFLAGS="${CPPFLAGS}" \
	  SAREL="${SAREL}" \
	  SADIR="${SADIR}" \
	  EXTRACFLAGS="${EXTRACFLAGS}"
.if defined(NO_NET)
SAMAKE+= NO_NET=""
.endif
.if defined(SA_ZLIB)
SAMAKE+= SA_ZLIB=""
.endif
.if defined(USE_LOADFILE)
SAMAKE+= USE_LOADFILE=""
.endif

${SALIB}:		.NOTMAIN __always_make_salib
	@echo making sure the stand-alone library is up to date...
	@${SAMAKE} libsa.a

clean::			.NOTMAIN __always_make_salib
	@echo cleaning the stand-alone library objects
	@${SAMAKE} clean
	-rmdir -p ${SADST}

beforedepend::		.NOTMAIN __always_make_salib
	@echo depending the standalone library objects
	@${SAMAKE} depend

__always_make_salib:	.NOTMAIN
	-mkdir -p ${SADST}

