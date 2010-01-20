
DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	ultrix_sysent.c ultrix_syscalls.c ultrix_syscall.h ultrix_syscallargs.h

${OBJS}: ${DEP}
	sh ../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
