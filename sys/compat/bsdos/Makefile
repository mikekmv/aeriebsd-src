
SYS=	../..
DEP=	syscalls.conf syscalls.master ${SYS}/kern/makesyscalls.sh
OBJS=	bsdos_sysent.c bsdos_syscalls.c \
	bsdos_syscall.h bsdos_syscallargs.h

${OBJS}: ${DEP}
	sh ${SYS}/kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}
