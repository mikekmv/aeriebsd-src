
.if make(obj)
SUBDIR= alpha amd64 arm hppa i386 m68k mips64 powerpc sh sparc sparc64 vax
.else
SUBDIR= ${MACHINE_ARCH}
.endif

.include <bsd.subdir.mk>
