
PROG=	flock

TESTS!=jot 15 1

.for a in ${TESTS}
t-${a}:
	./flock /tmp ${a}
REGRESS_TARGETS+=t-${a}
.endfor

.include <bsd.regress.mk>
