
DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	osf1_sysent.c osf1_syscalls.c osf1_syscall.h osf1_syscallargs.h

${OBJS}: ${DEP}
	sh ../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
