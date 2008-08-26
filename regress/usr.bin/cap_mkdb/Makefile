
CAP_MKDB?=	cap_mkdb

REGRESS_TARGETS=shortname
CLEANFILES+=	*.db

shortname:
	${CAP_MKDB} -if shortname ${.CURDIR}/shortname.in || case "$$?" in 1) exit 0;; esac && exit 1

.include <bsd.regress.mk>
