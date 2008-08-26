
CLEANFILES+= in in1 out xx -k xsort linecount fields

REGRESS_TARGETS=do-stests

# XXX - should be split up in this makefile, not in the script.
# XXX - script doesn't return non-0 on failure.
do-stests: ${.CURDIR}/stests
	sh ${.CURDIR}/stests

.include <bsd.regress.mk>
