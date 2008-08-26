
SUBDIR=	../ramdisk cdfs

.if	make(obj) || make(cleandir) || make(clean)
SUBDIR+= iso
.endif

.include <bsd.subdir.mk>
