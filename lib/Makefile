SUBDIR=	csu libarch libc libcompat libcurses \
	libedit libevent libelf libexpat \
	libform libkeynote libkvm libl libm libmenu \
	libocurses libossaudio libpanel libpcap libplot libpthread librpcsvc \
	libskey libssl libusbhid libutil libwrap liby libz

.include <bsd.own.mk>

.if (${KERBEROS5:L} == "yes")
SUBDIR+=libgssapi libkadm5srv libkadm5clnt libkrb5
.endif

.include <bsd.subdir.mk>
