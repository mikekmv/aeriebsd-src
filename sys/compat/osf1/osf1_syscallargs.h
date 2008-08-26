/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 *
 * created from
 *		OpenBSD: syscalls.master,v 1.11 2008/01/05 00:36:13 miod Exp 
 */

#ifdef	syscallarg
#undef	syscallarg
#endif

#define	syscallarg(x)							\
	union {								\
		register_t pad;						\
		struct { x datum; } le;					\
		struct {						\
			int8_t pad[ (sizeof (register_t) < sizeof (x))	\
				? 0					\
				: sizeof (register_t) - sizeof (x)];	\
			x datum;					\
		} be;							\
	}

struct osf1_sys_wait4_args {
	syscallarg(int) pid;
	syscallarg(int *) status;
	syscallarg(int) options;
	syscallarg(struct osf1_rusage *) rusage;
};

struct osf1_sys_mknod_args {
	syscallarg(char *) path;
	syscallarg(int) mode;
	syscallarg(int) dev;
};

struct osf1_sys_getfsstat_args {
	syscallarg(struct osf1_statfs *) buf;
	syscallarg(long) bufsize;
	syscallarg(int) flags;
};

struct osf1_sys_lseek_args {
	syscallarg(int) fd;
	syscallarg(off_t) offset;
	syscallarg(int) whence;
};

struct osf1_sys_mount_args {
	syscallarg(int) type;
	syscallarg(const char *) path;
	syscallarg(int) flags;
	syscallarg(caddr_t) data;
};

struct osf1_sys_unmount_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
};

struct osf1_sys_setuid_args {
	syscallarg(uid_t) uid;
};

struct osf1_sys_recvmsg_xopen_args {
	syscallarg(int) s;
	syscallarg(struct osf1_msghdr_xopen *) msg;
	syscallarg(int) flags;
};

struct osf1_sys_sendmsg_xopen_args {
	syscallarg(int) s;
	syscallarg(const struct osf1_msghdr_xopen *) msg;
	syscallarg(int) flags;
};

struct osf1_sys_access_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
};

struct osf1_sys_set_program_attributes_args {
	syscallarg(caddr_t) taddr;
	syscallarg(unsigned long) tsize;
	syscallarg(caddr_t) daddr;
	syscallarg(unsigned long) dsize;
};

struct osf1_sys_open_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
	syscallarg(int) mode;
};

struct osf1_sys_classcntl_args {
	syscallarg(int) opcode;
	syscallarg(int) arg1;
	syscallarg(int) arg2;
	syscallarg(int) arg3;
};

struct osf1_sys_ioctl_args {
	syscallarg(int) fd;
	syscallarg(int) com;
	syscallarg(caddr_t) data;
};

struct osf1_sys_reboot_args {
	syscallarg(int) opt;
};

struct osf1_sys_execve_args {
	syscallarg(char *) path;
	syscallarg(char *const *) argp;
	syscallarg(char *const *) envp;
};

struct osf1_sys_stat_args {
	syscallarg(char *) path;
	syscallarg(struct osf1_stat *) ub;
};

struct osf1_sys_lstat_args {
	syscallarg(char *) path;
	syscallarg(struct osf1_stat *) ub;
};

struct osf1_sys_mmap_args {
	syscallarg(caddr_t) addr;
	syscallarg(size_t) len;
	syscallarg(int) prot;
	syscallarg(int) flags;
	syscallarg(int) fd;
	syscallarg(off_t) pos;
};

struct osf1_sys_mprotect_args {
	syscallarg(void *) addr;
	syscallarg(size_t) len;
	syscallarg(int) prot;
};

struct osf1_sys_madvise_args {
	syscallarg(void *) addr;
	syscallarg(size_t) len;
	syscallarg(int) behav;
};

struct osf1_sys_setitimer_args {
	syscallarg(u_int) which;
	syscallarg(struct osf1_itimerval *) itv;
	syscallarg(struct osf1_itimerval *) oitv;
};

struct osf1_sys_fstat_args {
	syscallarg(int) fd;
	syscallarg(void *) sb;
};

struct osf1_sys_fcntl_args {
	syscallarg(int) fd;
	syscallarg(int) cmd;
	syscallarg(void *) arg;
};

struct osf1_sys_select_args {
	syscallarg(u_int) nd;
	syscallarg(fd_set *) in;
	syscallarg(fd_set *) ou;
	syscallarg(fd_set *) ex;
	syscallarg(struct osf1_timeval *) tv;
};

struct osf1_sys_socket_args {
	syscallarg(int) domain;
	syscallarg(int) type;
	syscallarg(int) protocol;
};

struct osf1_sys_gettimeofday_args {
	syscallarg(struct osf1_timeval *) tp;
	syscallarg(struct osf1_timezone *) tzp;
};

struct osf1_sys_getrusage_args {
	syscallarg(int) who;
	syscallarg(struct osf1_rusage *) rusage;
};

struct osf1_sys_readv_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_iovec *) iovp;
	syscallarg(u_int) iovcnt;
};

struct osf1_sys_writev_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_iovec *) iovp;
	syscallarg(u_int) iovcnt;
};

struct osf1_sys_settimeofday_args {
	syscallarg(struct osf1_timeval *) tv;
	syscallarg(struct osf1_timezone *) tzp;
};

struct osf1_sys_truncate_args {
	syscallarg(char *) path;
	syscallarg(off_t) length;
};

struct osf1_sys_ftruncate_args {
	syscallarg(int) fd;
	syscallarg(off_t) length;
};

struct osf1_sys_setgid_args {
	syscallarg(gid_t) gid;
};

struct osf1_sys_sendto_args {
	syscallarg(int) s;
	syscallarg(caddr_t) buf;
	syscallarg(size_t) len;
	syscallarg(int) flags;
	syscallarg(struct sockaddr *) to;
	syscallarg(int) tolen;
};

struct osf1_sys_socketpair_args {
	syscallarg(int) domain;
	syscallarg(int) type;
	syscallarg(int) protocol;
	syscallarg(int *) rsv;
};

struct osf1_sys_utimes_args {
	syscallarg(char *) path;
	syscallarg(const struct osf1_timeval *) tptr;
};

struct osf1_sys_getrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct rlimit *) rlp;
};

struct osf1_sys_setrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct rlimit *) rlp;
};

struct osf1_sys_sigaction_args {
	syscallarg(int) signum;
	syscallarg(struct osf1_sigaction *) nsa;
	syscallarg(struct osf1_sigaction *) osa;
};

struct osf1_sys_statfs_args {
	syscallarg(const char *) path;
	syscallarg(struct osf1_statfs *) buf;
	syscallarg(int) len;
};

struct osf1_sys_fstatfs_args {
	syscallarg(int) fd;
	syscallarg(struct osf1_statfs *) buf;
	syscallarg(int) len;
};

struct osf1_sys_uname_args {
	syscallarg(struct osf1_uname *) name;
};

struct osf1_sys_shmat_args {
	syscallarg(int) shmid;
	syscallarg(const void *) shmaddr;
	syscallarg(int) shmflg;
};

struct osf1_sys_shmctl_args {
	syscallarg(int) shmid;
	syscallarg(int) cmd;
	syscallarg(struct osf1_shmid_ds *) buf;
};

struct osf1_sys_shmdt_args {
	syscallarg(const void *) shmaddr;
};

struct osf1_sys_shmget_args {
	syscallarg(osf1_key_t) key;
	syscallarg(size_t) size;
	syscallarg(int) flags;
};

struct osf1_sys_sigaltstack_args {
	syscallarg(struct osf1_sigaltstack *) nss;
	syscallarg(struct osf1_sigaltstack *) oss;
};

struct osf1_sys_sysinfo_args {
	syscallarg(int) cmd;
	syscallarg(char *) buf;
	syscallarg(long) len;
};

struct osf1_sys_pathconf_args {
	syscallarg(char *) path;
	syscallarg(int) name;
};

struct osf1_sys_fpathconf_args {
	syscallarg(int) fd;
	syscallarg(int) name;
};

struct osf1_sys_usleep_thread_args {
	syscallarg(struct osf1_timeval *) sleep;
	syscallarg(struct osf1_timeval *) slept;
};

struct osf1_sys_setsysinfo_args {
	syscallarg(u_long) op;
	syscallarg(caddr_t) buffer;
	syscallarg(u_long) nbytes;
	syscallarg(caddr_t) arg;
	syscallarg(u_long) flag;
};

/*
 * System call prototypes.
 */

int	sys_nosys(struct proc *, void *, register_t *);
int	sys_exit(struct proc *, void *, register_t *);
int	sys_fork(struct proc *, void *, register_t *);
int	sys_read(struct proc *, void *, register_t *);
int	sys_write(struct proc *, void *, register_t *);
int	sys_close(struct proc *, void *, register_t *);
int	osf1_sys_wait4(struct proc *, void *, register_t *);
int	sys_link(struct proc *, void *, register_t *);
int	sys_unlink(struct proc *, void *, register_t *);
int	sys_chdir(struct proc *, void *, register_t *);
int	sys_fchdir(struct proc *, void *, register_t *);
int	osf1_sys_mknod(struct proc *, void *, register_t *);
int	sys_chmod(struct proc *, void *, register_t *);
int	sys_chown(struct proc *, void *, register_t *);
int	sys_obreak(struct proc *, void *, register_t *);
int	osf1_sys_getfsstat(struct proc *, void *, register_t *);
int	osf1_sys_lseek(struct proc *, void *, register_t *);
int	sys_getpid(struct proc *, void *, register_t *);
int	osf1_sys_mount(struct proc *, void *, register_t *);
int	osf1_sys_unmount(struct proc *, void *, register_t *);
int	osf1_sys_setuid(struct proc *, void *, register_t *);
int	sys_getuid(struct proc *, void *, register_t *);
int	osf1_sys_recvmsg_xopen(struct proc *, void *, register_t *);
int	osf1_sys_sendmsg_xopen(struct proc *, void *, register_t *);
int	osf1_sys_access(struct proc *, void *, register_t *);
int	sys_sync(struct proc *, void *, register_t *);
int	sys_kill(struct proc *, void *, register_t *);
int	sys_setpgid(struct proc *, void *, register_t *);
int	sys_dup(struct proc *, void *, register_t *);
int	sys_pipe(struct proc *, void *, register_t *);
int	osf1_sys_set_program_attributes(struct proc *, void *, register_t *);
int	osf1_sys_open(struct proc *, void *, register_t *);
int	sys_getgid(struct proc *, void *, register_t *);
int	sys_sigprocmask(struct proc *, void *, register_t *);
int	sys_getlogin(struct proc *, void *, register_t *);
int	sys_setlogin(struct proc *, void *, register_t *);
#ifdef	ACCOUNTING
int	sys_acct(struct proc *, void *, register_t *);
#else
#endif
int	osf1_sys_classcntl(struct proc *, void *, register_t *);
int	osf1_sys_ioctl(struct proc *, void *, register_t *);
int	osf1_sys_reboot(struct proc *, void *, register_t *);
int	sys_revoke(struct proc *, void *, register_t *);
int	sys_symlink(struct proc *, void *, register_t *);
int	sys_readlink(struct proc *, void *, register_t *);
int	osf1_sys_execve(struct proc *, void *, register_t *);
int	sys_umask(struct proc *, void *, register_t *);
int	sys_chroot(struct proc *, void *, register_t *);
int	sys_getpgrp(struct proc *, void *, register_t *);
int	compat_43_sys_getpagesize(struct proc *, void *, register_t *);
int	sys_vfork(struct proc *, void *, register_t *);
int	osf1_sys_stat(struct proc *, void *, register_t *);
int	osf1_sys_lstat(struct proc *, void *, register_t *);
int	osf1_sys_mmap(struct proc *, void *, register_t *);
int	sys_munmap(struct proc *, void *, register_t *);
int	osf1_sys_mprotect(struct proc *, void *, register_t *);
int	osf1_sys_madvise(struct proc *, void *, register_t *);
int	sys_getgroups(struct proc *, void *, register_t *);
int	sys_setgroups(struct proc *, void *, register_t *);
int	sys_setpgid(struct proc *, void *, register_t *);
int	osf1_sys_setitimer(struct proc *, void *, register_t *);
int	compat_43_sys_gethostname(struct proc *, void *, register_t *);
int	compat_43_sys_sethostname(struct proc *, void *, register_t *);
int	compat_43_sys_getdtablesize(struct proc *, void *, register_t *);
int	sys_dup2(struct proc *, void *, register_t *);
int	osf1_sys_fstat(struct proc *, void *, register_t *);
int	osf1_sys_fcntl(struct proc *, void *, register_t *);
int	osf1_sys_select(struct proc *, void *, register_t *);
int	sys_poll(struct proc *, void *, register_t *);
int	sys_fsync(struct proc *, void *, register_t *);
int	sys_setpriority(struct proc *, void *, register_t *);
int	osf1_sys_socket(struct proc *, void *, register_t *);
int	sys_connect(struct proc *, void *, register_t *);
int	compat_43_sys_accept(struct proc *, void *, register_t *);
int	sys_getpriority(struct proc *, void *, register_t *);
int	compat_43_sys_send(struct proc *, void *, register_t *);
int	compat_43_sys_recv(struct proc *, void *, register_t *);
int	sys_sigreturn(struct proc *, void *, register_t *);
int	sys_bind(struct proc *, void *, register_t *);
int	sys_setsockopt(struct proc *, void *, register_t *);
int	sys_listen(struct proc *, void *, register_t *);
int	sys_sigsuspend(struct proc *, void *, register_t *);
int	compat_43_sys_sigstack(struct proc *, void *, register_t *);
int	osf1_sys_gettimeofday(struct proc *, void *, register_t *);
int	osf1_sys_getrusage(struct proc *, void *, register_t *);
int	sys_getsockopt(struct proc *, void *, register_t *);
int	osf1_sys_readv(struct proc *, void *, register_t *);
int	osf1_sys_writev(struct proc *, void *, register_t *);
int	osf1_sys_settimeofday(struct proc *, void *, register_t *);
int	sys_fchown(struct proc *, void *, register_t *);
int	sys_fchmod(struct proc *, void *, register_t *);
int	compat_43_sys_recvfrom(struct proc *, void *, register_t *);
int	sys_setreuid(struct proc *, void *, register_t *);
int	sys_setregid(struct proc *, void *, register_t *);
int	sys_rename(struct proc *, void *, register_t *);
int	osf1_sys_truncate(struct proc *, void *, register_t *);
int	osf1_sys_ftruncate(struct proc *, void *, register_t *);
int	osf1_sys_setgid(struct proc *, void *, register_t *);
int	osf1_sys_sendto(struct proc *, void *, register_t *);
int	sys_shutdown(struct proc *, void *, register_t *);
int	osf1_sys_socketpair(struct proc *, void *, register_t *);
int	sys_mkdir(struct proc *, void *, register_t *);
int	sys_rmdir(struct proc *, void *, register_t *);
int	osf1_sys_utimes(struct proc *, void *, register_t *);
int	compat_43_sys_getpeername(struct proc *, void *, register_t *);
int	compat_43_sys_gethostid(struct proc *, void *, register_t *);
int	compat_43_sys_sethostid(struct proc *, void *, register_t *);
int	osf1_sys_getrlimit(struct proc *, void *, register_t *);
int	osf1_sys_setrlimit(struct proc *, void *, register_t *);
int	sys_setsid(struct proc *, void *, register_t *);
int	compat_43_sys_quota(struct proc *, void *, register_t *);
int	compat_43_sys_getsockname(struct proc *, void *, register_t *);
int	osf1_sys_sigaction(struct proc *, void *, register_t *);
int	compat_43_sys_getdirentries(struct proc *, void *, register_t *);
int	osf1_sys_statfs(struct proc *, void *, register_t *);
int	osf1_sys_fstatfs(struct proc *, void *, register_t *);
int	compat_09_sys_getdomainname(struct proc *, void *, register_t *);
int	compat_09_sys_setdomainname(struct proc *, void *, register_t *);
int	osf1_sys_uname(struct proc *, void *, register_t *);
int	sys_lchown(struct proc *, void *, register_t *);
int	osf1_sys_shmat(struct proc *, void *, register_t *);
int	osf1_sys_shmctl(struct proc *, void *, register_t *);
int	osf1_sys_shmdt(struct proc *, void *, register_t *);
int	osf1_sys_shmget(struct proc *, void *, register_t *);
int	sys_getsid(struct proc *, void *, register_t *);
int	osf1_sys_sigaltstack(struct proc *, void *, register_t *);
int	osf1_sys_sysinfo(struct proc *, void *, register_t *);
int	osf1_sys_pathconf(struct proc *, void *, register_t *);
int	osf1_sys_fpathconf(struct proc *, void *, register_t *);
int	osf1_sys_usleep_thread(struct proc *, void *, register_t *);
int	osf1_sys_setsysinfo(struct proc *, void *, register_t *);
