
REGRESS_TARGETS=pt li	# lm co mo la ar ci big
CLEANFILES+=pt.pl li.pl

.SUFFIXES: .pl .awk

.awk.pl:
	awk -f ${.IMPSRC} > $@

pt li: $@.pl
	plot -Tps $@.pl | diff ${.CURDIR}/$@.ps -

.PHONY: pt

.include <bsd.regress.mk>
