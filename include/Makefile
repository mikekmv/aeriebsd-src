#	from:	@(#)Makefile	5.45.1.1 (Berkeley) 5/6/91

# The ``rm -rf''s used below are safe because rm doesn't follow symbolic
# links.

.include <bsd.own.mk>

# Missing: mp.h
FILES=	a.out.h ar.h assert.h bitstring.h blf.h bm.h bsd_auth.h \
	cast.h complex.h cpio.h ctype.h curses.h \
	db.h dbm.h des.h dirent.h disktab.h dlfcn.h dwarf.h \
	elf_abi.h err.h errno.h fnmatch.h fstab.h fts.h ftw.h \
	getopt.h glob.h grp.h ifaddrs.h inttypes.h iso646.h \
	kvm.h langinfo.h libgen.h limits.h locale.h login_cap.h \
	malloc.h math.h md4.h md5.h memory.h mpool.h \
	ndbm.h netdb.h netgroup.h nlist.h nl_types.h \
	ohash.h paths.h poll.h pwd.h \
	ranlib.h re_comp.h readpassphrase.h regex.h resolv.h rmd160.h \
	search.h setjmp.h sgtty.h sha1.h sha2.h skipjack.h signal.h \
	stab.h stdbool.h stddef.h stdio.h stdlib.h string.h \
	strings.h struct.h sysexits.h \
	tar.h time.h ttyent.h tzfile.h unistd.h utime.h utmp.h \
	vis.h wchar.h wctype.h

FILES+=	link.h link_aout.h link_elf.h

.if (${MACHINE_ARCH} != "vax")
FILES+= ieeefp.h
.endif

MFILES=	float.h frame.h 
LFILES=	fcntl.h syslog.h termios.h stdarg.h stdint.h varargs.h

DIRS=	arpa protocols rpc rpcsvc
LDIRS=	altq crypto ddb dev isofs milfs miscfs net netatalk netinet \
	netinet6 netmpls netnatm net80211 netbt nfs scsi sys ufs uvm xfs
ADIRS=	alpha amd64 arm armish hp300 hppa hppa64 i386 \
	landisk m68k mac68k macppc mips64 powerpc sgi \
	sh sparc sparc64 vax zaurus

# Directories with an includes target
RDIRS=	../lib/libpthread ../lib/libcompat ../lib/libcurses \
	../lib/libelf ../lib/libform ../lib/libssl ../lib/libmenu \
	../lib/libocurses ../lib/libossaudio ../lib/libpanel ../lib/librpcsvc \
	../lib/libskey ../lib/libedit ../lib/libedit/readline ../lib/libexpat \
	../lib/libpcap ../lib/libplot ../lib/libutil ../lib/libusbhid \
	../lib/libwrap ../lib/libz ../lib/libkeynote ../lib/libevent \
	../usr.bin/lex ../sys/arch/${MACHINE}

# Places using Makefile that needs a prerequisite target met before includes
PRDIRS=

# Directories with an includes target that use Makefile.bsd-wrapper
WDIRS=	../usr.sbin/httpd

# Places using Makefile.bsd-wrapper that needs a prerequisite target met
# before includes
PWDIRS=

NOOBJ=	noobj

# Change SYS_INCLUDE in bsd.own.mk to "symlinks" if you don't want copies
SYS_INCLUDE?=	copies
.if ${KERBEROS5:L} == "yes"
RDIRS+= ../lib/libkrb5 ../lib/libgssapi ../lib/libkadm5srv
.endif

prereq:
	@for i in ${PRDIRS}; do \
		echo preparing in ${.CURDIR}/$$i; \
		cd ${.CURDIR}/$$i && ${MAKE} prereq; \
	done
	@for i in ${PWDIRS}; do \
		echo preparing in ${.CURDIR}/$$i; \
		cd ${.CURDIR}/$$i && ${MAKE} -f Makefile.bsd-wrapper prereq; \
	done

includes:
	@echo installing ${FILES}
	@for i in ${FILES}; do \
		cmp -s $$i ${DESTDIR}/usr/include/$$i || \
		    ${INSTALL} ${INSTALL_COPY} -m 444 $$i ${DESTDIR}/usr/include/$$i; \
	done
	@echo installing ${DIRS}
	@for i in ${DIRS}; do \
		${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 755 \
			${DESTDIR}/usr/include/$$i; \
		cd ${.CURDIR}/$$i && for j in *.[ih]; do \
			cmp -s $$j ${DESTDIR}/usr/include/$$i/$$j || \
			${INSTALL} ${INSTALL_COPY} -m 444 $$j ${DESTDIR}/usr/include/$$i/$$j; \
		done; \
	done
	@rm -rf ${DESTDIR}/usr/include/openssl ${DESTDIR}/usr/include/ssl \
		${DESTDIR}/usr/libdata/perl5/site_perl/${MACHINE_ARCH}-openbsd/ssl \
		${DESTDIR}/usr/libdata/perl5/site_perl/${MACHINE_ARCH}-openbsd/openssl
	@mkdir ${DESTDIR}/usr/include/openssl
	@ln -sf openssl ${DESTDIR}/usr/include/ssl
	@echo installing ${LFILES}
	@for i in ${LFILES}; do \
		rm -f ${DESTDIR}/usr/include/$$i && \
		ln -s sys/$$i ${DESTDIR}/usr/include/$$i; \
	done
	@echo installing ${MFILES}
	@for i in ${MFILES}; do \
		rm -f ${DESTDIR}/usr/include/$$i && \
		ln -s machine/$$i ${DESTDIR}/usr/include/$$i; \
	done
	chown -R ${BINOWN}:${BINGRP} ${DESTDIR}/usr/include
	find ${DESTDIR}/usr/include -type f -print0 | \
		xargs -0r chmod a=r
	find ${DESTDIR}/usr/include -type d -print0 | \
		xargs -0r chmod u=rwx,go=rx
	@for i in ${RDIRS}; do \
		echo installing in ${.CURDIR}/$$i; \
		cd ${.CURDIR}/$$i && ${MAKE} includes; \
	done
	@for i in ${WDIRS}; do \
		echo installing in ${.CURDIR}/$$i; \
		cd ${.CURDIR}/$$i && ${MAKE} -f Makefile.bsd-wrapper includes; \
	done

copies:
	@echo copies: ${LDIRS} ${ADIRS}
	@for i in ${LDIRS}; do \
		rm -rf ${DESTDIR}/usr/include/$$i && \
		${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 755 \
			${DESTDIR}/usr/include/$$i ; \
	done
	cd ../sys; \
	pax -rw -pa -L \
	    `find ${LDIRS} -follow -type f -name '*.h' \
	    '!' -path 'dev/microcode/*' -print` ${DESTDIR}/usr/include
	@for i in ${ADIRS}; do \
		rm -rf ${DESTDIR}/usr/include/$$i && \
		${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 755 \
			${DESTDIR}/usr/include/$$i ; \
	done
	cd ../sys/arch; \
	pax -rw -pa -L -s "|/include||" \
	    `find ${ADIRS} -follow -type f -name '*.h' \
	    -path '*/include/*' -print` ${DESTDIR}/usr/include
	ln -hsf ${MACHINE} ${DESTDIR}/usr/include/machine; \

symlinks:
	@echo symlinks: ${LDIRS} ${ADIRS}
	@for i in ${LDIRS}; do \
		rm -rf ${DESTDIR}/usr/include/$$i && \
		ln -s /sys/$$i ${DESTDIR}/usr/include/$$i; \
	done
	@for i in ${ADIRS}; do \
		rm -rf ${DESTDIR}/usr/include/$$i && \
		ln -s /sys/arch/$$i/include ${DESTDIR}/usr/include/$$i; \
	done
	ln -hsf ${MACHINE} ${DESTDIR}/usr/include/machine

includes: ${SYS_INCLUDE}

.PHONY: prereq includes copies symlink
.include <bsd.prog.mk>
