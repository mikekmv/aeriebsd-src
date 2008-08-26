#
#	NOTE: $S must correspond to the top of the `sys' tree

060SPSRCDIR=	$S/arch/m68k/060sp

060SPOBJDIR!=	cd $(060SPSRCDIR); \
	printf "xxx: .MAKE\n\t@echo \$${.OBJDIR}\n" | $(MAKE) -s -f-

060SPOBJ=	$(060SPOBJDIR)/060sp.o

060SPMAKE= \
	cd ${060SPSRCDIR} && MAKEOBJDIR=${060SPOBJDIR} ${MAKE}

$(060SPOBJ):	.NOTMAIN __always_make_060sp
	@echo making sure the 060sp is up to date...
	@${060SPMAKE}

clean::		.NOTMAIN __always_make_060sp
	@echo cleaning the 060sp objects
	@${060SPMAKE} clean

060SP!=	printf "\#ifdef M060SP\n${060SPOBJ}\n\#endif\n" | cpp -P -undef ${CPPFLAGS:M-DM060SP}

__always_make_060sp: .NOTMAIN
