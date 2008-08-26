
PROG=dtors

REGRESS_TARGETS=dtor-exit dtor-return

dtor-exit: dtors
	./dtors

dtor-return: dtors
	./dtors -f

.include <bsd.regress.mk>
