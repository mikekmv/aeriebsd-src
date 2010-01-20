
DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	linux_sysent.c linux_syscalls.c linux_syscall.h linux_syscallargs.h

${OBJS}: ${DEP}
	sh ../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
