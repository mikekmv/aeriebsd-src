SUBDIR=	special notes

.if	make(obj)
SUBDIR+=crunch
SUBDIR+=alpha amd64 armish hp300 hppa i386 landisk mac68k macppc \
	sgi sparc sparc64 vax zaurus
.elif exists(${MACHINE})
SUBDIR+=${MACHINE}
.endif

.if	make(clean) || make(cleandir)
SUBDIR+=crunch
.endif

#all: crunch-tools _SUBDIRUSE

crunch-tools:
	cd $(.CURDIR)/crunch; $(MAKE) obj; \
	 $(MAKE) depend && ${MAKE} && ${SUDO} ${MAKE} DESTDIR= install

.include <bsd.subdir.mk>
