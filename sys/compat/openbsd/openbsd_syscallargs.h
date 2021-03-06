/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 *
 * created from
 *	ABSD: syscalls.master,v 1.4 2010/02/12 14:25:06 mickey Exp 
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

struct openbsd_sys_open_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
	syscallarg(mode_t) mode;
};

struct openbsd_sys_link_args {
	syscallarg(char *) path;
	syscallarg(char *) link;
};

struct openbsd_sys_unlink_args {
	syscallarg(char *) path;
};

struct openbsd_sys_chdir_args {
	syscallarg(char *) path;
};

struct openbsd_sys_mknod_args {
	syscallarg(char *) path;
	syscallarg(mode_t) mode;
	syscallarg(dev_t) dev;
};

struct openbsd_sys_chmod_args {
	syscallarg(char *) path;
	syscallarg(mode_t) mode;
};

struct openbsd_sys_chown_args {
	syscallarg(char *) path;
	syscallarg(uid_t) uid;
	syscallarg(gid_t) gid;
};

struct openbsd_sys_mount_args {
	syscallarg(char *) type;
	syscallarg(char *) path;
	syscallarg(int) flags;
	syscallarg(void *) data;
};

struct openbsd_sys_unmount_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
};

struct openbsd_sys_access_args {
	syscallarg(char *) path;
	syscallarg(int) flags;
};

struct openbsd_sys_chflags_args {
	syscallarg(char *) path;
	syscallarg(u_int) flags;
};

struct openbsd_sys_revoke_args {
	syscallarg(char *) path;
};

struct openbsd_sys_symlink_args {
	syscallarg(char *) path;
	syscallarg(char *) link;
};

struct openbsd_sys_readlink_args {
	syscallarg(char *) path;
	syscallarg(char *) buf;
	syscallarg(size_t) count;
};

struct openbsd_sys_execve_args {
	syscallarg(char *) path;
	syscallarg(char *const *) argp;
	syscallarg(char *const *) envp;
};

struct openbsd_sys_chroot_args {
	syscallarg(char *) path;
};

struct openbsd_sys_rename_args {
	syscallarg(char *) from;
	syscallarg(char *) to;
};

struct openbsd_sys_mkfifo_args {
	syscallarg(char *) path;
	syscallarg(mode_t) mode;
};

struct openbsd_sys_mkdir_args {
	syscallarg(char *) path;
	syscallarg(mode_t) mode;
};

struct openbsd_sys_rmdir_args {
	syscallarg(char *) path;
};

struct openbsd_sys_utimes_args {
	syscallarg(char *) path;
	syscallarg(const struct timeval *) tptr;
};

struct openbsd_sys_getfh_args {
	syscallarg(char *) fname;
	syscallarg(fhandle_t *) fhp;
};

struct openbsd_sys_pathconf_args {
	syscallarg(char *) path;
	syscallarg(int) name;
};

struct openbsd_sys_truncate_args {
	syscallarg(char *) path;
	syscallarg(int) pad;
	syscallarg(off_t) length;
};

struct openbsd_sys_lchown_args {
	syscallarg(char *) path;
	syscallarg(uid_t) uid;
	syscallarg(gid_t) gid;
};

struct openbsd_sys_stat_args {
	syscallarg(char *) path;
	syscallarg(struct stat *) ub;
};

struct openbsd_sys_lstat_args {
	syscallarg(char *) path;
	syscallarg(struct stat *) ub;
};

/*
 * System call prototypes.
 */

int	sys_nosys(struct proc *, void *, register_t *);
int	sys_exit(struct proc *, void *, register_t *);
int	sys_fork(struct proc *, void *, register_t *);
int	sys_read(struct proc *, void *, register_t *);
int	sys_write(struct proc *, void *, register_t *);
int	openbsd_sys_open(struct proc *, void *, register_t *);
int	sys_close(struct proc *, void *, register_t *);
int	sys_wait4(struct proc *, void *, register_t *);
int	openbsd_sys_link(struct proc *, void *, register_t *);
int	openbsd_sys_unlink(struct proc *, void *, register_t *);
int	openbsd_sys_chdir(struct proc *, void *, register_t *);
int	sys_fchdir(struct proc *, void *, register_t *);
int	openbsd_sys_mknod(struct proc *, void *, register_t *);
int	openbsd_sys_chmod(struct proc *, void *, register_t *);
int	openbsd_sys_chown(struct proc *, void *, register_t *);
int	compat_43_sys_obreak(struct proc *, void *, register_t *);
int	sys_getpid(struct proc *, void *, register_t *);
int	openbsd_sys_mount(struct proc *, void *, register_t *);
int	openbsd_sys_unmount(struct proc *, void *, register_t *);
int	sys_setuid(struct proc *, void *, register_t *);
int	sys_getuid(struct proc *, void *, register_t *);
int	sys_geteuid(struct proc *, void *, register_t *);
#ifdef PTRACE
int	sys_ptrace(struct proc *, void *, register_t *);
#else
#endif
int	sys_recvmsg(struct proc *, void *, register_t *);
int	sys_sendmsg(struct proc *, void *, register_t *);
int	sys_recvfrom(struct proc *, void *, register_t *);
int	sys_accept(struct proc *, void *, register_t *);
int	sys_getpeername(struct proc *, void *, register_t *);
int	sys_getsockname(struct proc *, void *, register_t *);
int	openbsd_sys_access(struct proc *, void *, register_t *);
int	openbsd_sys_chflags(struct proc *, void *, register_t *);
int	sys_fchflags(struct proc *, void *, register_t *);
int	sys_sync(struct proc *, void *, register_t *);
int	sys_kill(struct proc *, void *, register_t *);
int	sys_getppid(struct proc *, void *, register_t *);
int	sys_dup(struct proc *, void *, register_t *);
int	compat_43_sys_opipe(struct proc *, void *, register_t *);
int	sys_getegid(struct proc *, void *, register_t *);
int	sys_profil(struct proc *, void *, register_t *);
#ifdef KTRACE
int	sys_ktrace(struct proc *, void *, register_t *);
#else
#endif
int	sys_sigaction(struct proc *, void *, register_t *);
int	sys_getgid(struct proc *, void *, register_t *);
int	sys_sigprocmask(struct proc *, void *, register_t *);
int	sys_getlogin(struct proc *, void *, register_t *);
int	sys_setlogin(struct proc *, void *, register_t *);
int	sys_sigpending(struct proc *, void *, register_t *);
int	compat_44_sys_osigaltstack(struct proc *, void *, register_t *);
int	sys_ioctl(struct proc *, void *, register_t *);
int	sys_reboot(struct proc *, void *, register_t *);
int	openbsd_sys_revoke(struct proc *, void *, register_t *);
int	openbsd_sys_symlink(struct proc *, void *, register_t *);
int	openbsd_sys_readlink(struct proc *, void *, register_t *);
int	openbsd_sys_execve(struct proc *, void *, register_t *);
int	sys_umask(struct proc *, void *, register_t *);
int	openbsd_sys_chroot(struct proc *, void *, register_t *);
int	sys_vfork(struct proc *, void *, register_t *);
int	sys_munmap(struct proc *, void *, register_t *);
int	sys_mprotect(struct proc *, void *, register_t *);
int	sys_madvise(struct proc *, void *, register_t *);
int	sys_mincore(struct proc *, void *, register_t *);
int	sys_getgroups(struct proc *, void *, register_t *);
int	sys_setgroups(struct proc *, void *, register_t *);
int	sys_getpgrp(struct proc *, void *, register_t *);
int	sys_setpgid(struct proc *, void *, register_t *);
int	sys_setitimer(struct proc *, void *, register_t *);
int	sys_getitimer(struct proc *, void *, register_t *);
int	sys_dup2(struct proc *, void *, register_t *);
int	sys_fcntl(struct proc *, void *, register_t *);
int	sys_select(struct proc *, void *, register_t *);
int	sys_fsync(struct proc *, void *, register_t *);
int	sys_setpriority(struct proc *, void *, register_t *);
int	sys_socket(struct proc *, void *, register_t *);
int	sys_connect(struct proc *, void *, register_t *);
int	sys_getpriority(struct proc *, void *, register_t *);
int	sys_sigreturn(struct proc *, void *, register_t *);
int	sys_bind(struct proc *, void *, register_t *);
int	sys_setsockopt(struct proc *, void *, register_t *);
int	sys_listen(struct proc *, void *, register_t *);
int	sys_sigsuspend(struct proc *, void *, register_t *);
int	sys_gettimeofday(struct proc *, void *, register_t *);
int	sys_getrusage(struct proc *, void *, register_t *);
int	sys_getsockopt(struct proc *, void *, register_t *);
int	sys_readv(struct proc *, void *, register_t *);
int	sys_writev(struct proc *, void *, register_t *);
int	sys_settimeofday(struct proc *, void *, register_t *);
int	sys_fchown(struct proc *, void *, register_t *);
int	sys_fchmod(struct proc *, void *, register_t *);
int	sys_setreuid(struct proc *, void *, register_t *);
int	sys_setregid(struct proc *, void *, register_t *);
int	openbsd_sys_rename(struct proc *, void *, register_t *);
int	sys_flock(struct proc *, void *, register_t *);
int	openbsd_sys_mkfifo(struct proc *, void *, register_t *);
int	sys_sendto(struct proc *, void *, register_t *);
int	sys_shutdown(struct proc *, void *, register_t *);
int	sys_socketpair(struct proc *, void *, register_t *);
int	openbsd_sys_mkdir(struct proc *, void *, register_t *);
int	openbsd_sys_rmdir(struct proc *, void *, register_t *);
int	openbsd_sys_utimes(struct proc *, void *, register_t *);
int	sys_adjtime(struct proc *, void *, register_t *);
int	sys_setsid(struct proc *, void *, register_t *);
int	sys_quotactl(struct proc *, void *, register_t *);
#if defined(NFSCLIENT) || defined(NFSSERVER)
int	sys_nfssvc(struct proc *, void *, register_t *);
#else
#endif
int	openbsd_sys_getfh(struct proc *, void *, register_t *);
int	sys_sysarch(struct proc *, void *, register_t *);
int	sys_pread(struct proc *, void *, register_t *);
int	sys_pwrite(struct proc *, void *, register_t *);
int	sys_setgid(struct proc *, void *, register_t *);
int	sys_setegid(struct proc *, void *, register_t *);
int	sys_seteuid(struct proc *, void *, register_t *);
#ifdef LFS
int	lfs_bmapv(struct proc *, void *, register_t *);
int	lfs_markv(struct proc *, void *, register_t *);
int	lfs_segclean(struct proc *, void *, register_t *);
int	lfs_segwait(struct proc *, void *, register_t *);
#else
#endif
int	openbsd_sys_pathconf(struct proc *, void *, register_t *);
int	sys_fpathconf(struct proc *, void *, register_t *);
int	sys_swapctl(struct proc *, void *, register_t *);
int	sys_getrlimit(struct proc *, void *, register_t *);
int	sys_setrlimit(struct proc *, void *, register_t *);
int	sys_getdirentries(struct proc *, void *, register_t *);
int	sys_mmap(struct proc *, void *, register_t *);
int	sys_lseek(struct proc *, void *, register_t *);
int	openbsd_sys_truncate(struct proc *, void *, register_t *);
int	sys_ftruncate(struct proc *, void *, register_t *);
int	sys___sysctl(struct proc *, void *, register_t *);
int	sys_mlock(struct proc *, void *, register_t *);
int	sys_munlock(struct proc *, void *, register_t *);
int	sys_futimes(struct proc *, void *, register_t *);
int	sys_getpgid(struct proc *, void *, register_t *);
int	sys_xfspioctl(struct proc *, void *, register_t *);
#ifdef LKM
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
int	sys_lkmnosys(struct proc *, void *, register_t *);
#else	/* !LKM */
#endif	/* !LKM */
#ifdef SYSVSEM
int	sys_semget(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVMSG
int	sys_msgget(struct proc *, void *, register_t *);
int	sys_msgsnd(struct proc *, void *, register_t *);
int	sys_msgrcv(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVSHM
int	sys_shmat(struct proc *, void *, register_t *);
int	sys_shmdt(struct proc *, void *, register_t *);
#else
#endif
int	sys_clock_gettime(struct proc *, void *, register_t *);
int	sys_clock_settime(struct proc *, void *, register_t *);
int	sys_clock_getres(struct proc *, void *, register_t *);
int	sys_nanosleep(struct proc *, void *, register_t *);
int	sys_minherit(struct proc *, void *, register_t *);
int	sys_rfork(struct proc *, void *, register_t *);
int	sys_poll(struct proc *, void *, register_t *);
int	sys_issetugid(struct proc *, void *, register_t *);
int	openbsd_sys_lchown(struct proc *, void *, register_t *);
int	sys_getsid(struct proc *, void *, register_t *);
int	sys_msync(struct proc *, void *, register_t *);
int	sys_pipe(struct proc *, void *, register_t *);
int	sys_fhopen(struct proc *, void *, register_t *);
int	sys_preadv(struct proc *, void *, register_t *);
int	sys_pwritev(struct proc *, void *, register_t *);
int	sys_kqueue(struct proc *, void *, register_t *);
int	sys_kevent(struct proc *, void *, register_t *);
int	sys_mlockall(struct proc *, void *, register_t *);
int	sys_munlockall(struct proc *, void *, register_t *);
int	sys_getpeereid(struct proc *, void *, register_t *);
int	sys_getresuid(struct proc *, void *, register_t *);
int	sys_setresuid(struct proc *, void *, register_t *);
int	sys_getresgid(struct proc *, void *, register_t *);
int	sys_setresgid(struct proc *, void *, register_t *);
int	sys_mquery(struct proc *, void *, register_t *);
int	sys_closefrom(struct proc *, void *, register_t *);
int	sys_sigaltstack(struct proc *, void *, register_t *);
#ifdef SYSVSHM
int	sys_shmget(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVSEM
int	sys_semop(struct proc *, void *, register_t *);
#else
#endif
int	openbsd_sys_stat(struct proc *, void *, register_t *);
int	sys_fstat(struct proc *, void *, register_t *);
int	openbsd_sys_lstat(struct proc *, void *, register_t *);
int	sys_fhstat(struct proc *, void *, register_t *);
#ifdef SYSVSEM
int	sys___semctl(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVSHM
int	sys_shmctl(struct proc *, void *, register_t *);
#else
#endif
#ifdef SYSVMSG
int	sys_msgctl(struct proc *, void *, register_t *);
#else
#endif
int	sys_sched_yield(struct proc *, void *, register_t *);
#ifdef RTHREADS
int	sys_getthrid(struct proc *, void *, register_t *);
int	sys_thrsleep(struct proc *, void *, register_t *);
int	sys_thrwakeup(struct proc *, void *, register_t *);
int	sys_threxit(struct proc *, void *, register_t *);
int	sys_thrsigdivert(struct proc *, void *, register_t *);
#else
#endif
int	sys___getcwd(struct proc *, void *, register_t *);
int	sys_adjfreq(struct proc *, void *, register_t *);
int	sys_getfsstat(struct proc *, void *, register_t *);
int	sys_statfs(struct proc *, void *, register_t *);
int	sys_fstatfs(struct proc *, void *, register_t *);
int	sys_fhstatfs(struct proc *, void *, register_t *);
