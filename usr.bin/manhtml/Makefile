
.include <bsd.own.mk>

HTML0DIR=${HTMLDIR:S/\/man//}
GENFILES=index.html
CLEANFILES+=${GENFILES}

index.html: ${.CURDIR}/../../share/man/man0/intro.0
	@echo "manhtml $? > ${.TARGET}"
	@${MANHTMLOFF} -v OSNAME=`uname -s` -v OSREL=${OSREV} \
	    $? > ${.TARGET} || (rm -f ${.TARGET}; false)

all:: ${GENFILES}

beforeinstall: ${GENFILES}
	for i in ${GENFILES}; do \
		install -C -o root -g wheel -m 0444 $$i \
		    ${DESTDIR}/${HTML0DIR}; \
	done

.include <bsd.man.mk>
.include <bsd.obj.mk>
.include <bsd.subdir.mk>
