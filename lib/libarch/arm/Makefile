
MAN+=	arm_sync_icache.2 arm_drain_writebuf.2
MANSUBDIR=armish zaurus

.if ${MACHINE} == "armish" || ${MACHINE} == "zaurus"
NOPIC=
SRCS+=	arm_sync_icache.c arm_drain_writebuf.c
.include <bsd.lib.mk>
.else
NOPROG=
.include <bsd.prog.mk>
.endif
