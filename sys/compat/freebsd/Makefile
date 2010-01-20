
SYS=	../..
DEP=	syscalls.conf syscalls.master ${SYS}/kern/makesyscalls.sh
OBJS=	freebsd_sysent.c freebsd_syscalls.c \
	freebsd_syscall.h freebsd_syscallargs.h

${OBJS}: ${DEP}
	sh ${SYS}/kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
