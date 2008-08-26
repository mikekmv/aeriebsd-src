#
#	NOTE: $S must correspond to the top of the `sys' tree

FPSPSRCDIR=	$S/arch/m68k/fpsp

FPSPOBJDIR!=	cd $(FPSPSRCDIR); \
	printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | $(MAKE) -s -f-

FPSPOBJ=	$(FPSPOBJDIR)/fpsp.o

FPSPMAKE= \
	cd ${FPSPSRCDIR} && MAKEOBJDIR=${FPSPOBJDIR} ${MAKE}

$(FPSPOBJ):		.NOTMAIN __always_make_fpsp
	@echo making sure the fpsp is up to date...
	@${FPSPMAKE}

clean::			.NOTMAIN __always_make_fpsp
	@echo cleaning the fpsp objects
	@${FPSPMAKE} clean

FPSP!=	printf "\#ifdef FPSP\n${FPSPOBJ}\n\#endif\n" | ${CPP} -P -undef ${CPPFLAGS:M-DFPSP}

__always_make_fpsp: .NOTMAIN
