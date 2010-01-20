
DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	svr4_sysent.c svr4_syscalls.c svr4_syscall.h svr4_syscallargs.h

${OBJS}: ${DEP}
	sh ../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
