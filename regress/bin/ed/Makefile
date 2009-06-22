
SHELL=	/bin/sh
ED=	${DESTDIR}/bin/ed
REGRESS_TARGETS=check-ed
CLEANFILES=*.ed *.red *.[oz]

check-ed: build-ed test-ed
	@cd ${.OBJDIR}
	@if grep -h '\*\*\*' errs.o scripts.o; then :; else \
		echo "tests completed successfully."; \
	fi

build-ed: ${.CURDIR}/mkscripts.sh *.t *err
	@cd ${.OBJDIR}
	lndir -e Makefile -e CVS -e obj ${.CURDIR} .
	@if [ -f errs.o ]; then :; else \
		echo "building test scripts for $(ED) ..."; \
		$(SHELL) mkscripts.sh $(ED); \
	fi

test-ed: ${.CURDIR}/ckscripts.sh
	@cd ${.OBJDIR}
	@echo testing $(ED) ...
	@$(SHELL) ckscripts.sh $(ED)

.include <bsd.regress.mk>
