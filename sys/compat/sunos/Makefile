
DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	sunos_sysent.c sunos_syscalls.c sunos_syscall.h sunos_syscallargs.h

${OBJS}: ${DEP}
	sh ../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
