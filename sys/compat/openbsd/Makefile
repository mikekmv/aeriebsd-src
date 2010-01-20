
SYS=	../..
DEP=	syscalls.conf syscalls.master ${SYS}/kern/makesyscalls.sh
OBJS=	openbsd_sysent.c openbsd_syscalls.c \
	openbsd_syscall.h openbsd_syscallargs.h

${OBJS}: ${DEP}
	sh ${SYS}/kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
