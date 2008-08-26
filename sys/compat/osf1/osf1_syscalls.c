/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 *
 * created from
 *		OpenBSD: syscalls.master,v 1.11 2008/01/05 00:36:13 miod Exp 
 */

char *osf1_syscallnames[] = {
	"syscall",			/* 0 = syscall */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"#5 (unimplemented old open)",		/* 5 = unimplemented old open */
	"close",			/* 6 = close */
	"wait4",			/* 7 = wait4 */
	"#8 (unimplemented old creat)",		/* 8 = unimplemented old creat */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"#11 (unimplemented execv)",		/* 11 = unimplemented execv */
	"chdir",			/* 12 = chdir */
	"fchdir",			/* 13 = fchdir */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown */
	"obreak",			/* 17 = obreak */
	"getfsstat",			/* 18 = getfsstat */
	"lseek",			/* 19 = lseek */
	"getpid",			/* 20 = getpid */
	"mount",			/* 21 = mount */
	"unmount",			/* 22 = unmount */
	"setuid",			/* 23 = setuid */
	"getuid",			/* 24 = getuid */
	"#25 (unimplemented exec_with_loader)",		/* 25 = unimplemented exec_with_loader */
	"#26 (unimplemented ptrace)",		/* 26 = unimplemented ptrace */
	"recvmsg_xopen",			/* 27 = recvmsg_xopen */
	"sendmsg_xopen",			/* 28 = sendmsg_xopen */
	"#29 (unimplemented recvfrom)",		/* 29 = unimplemented recvfrom */
	"#30 (unimplemented accept)",		/* 30 = unimplemented accept */
	"#31 (unimplemented getpeername)",		/* 31 = unimplemented getpeername */
	"#32 (unimplemented getsockname)",		/* 32 = unimplemented getsockname */
	"access",			/* 33 = access */
	"#34 (unimplemented chflags)",		/* 34 = unimplemented chflags */
	"#35 (unimplemented fchflags)",		/* 35 = unimplemented fchflags */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"#38 (unimplemented old stat)",		/* 38 = unimplemented old stat */
	"setpgid",			/* 39 = setpgid */
	"#40 (unimplemented old lstat)",		/* 40 = unimplemented old lstat */
	"dup",			/* 41 = dup */
	"pipe",			/* 42 = pipe */
	"set_program_attributes",			/* 43 = set_program_attributes */
	"#44 (unimplemented profil)",		/* 44 = unimplemented profil */
	"open",			/* 45 = open */
	"#46 (obsolete sigaction)",		/* 46 = obsolete sigaction */
	"getgid",			/* 47 = getgid */
	"sigprocmask",			/* 48 = sigprocmask */
	"getlogin",			/* 49 = getlogin */
	"setlogin",			/* 50 = setlogin */
#ifdef	ACCOUNTING
	"acct",			/* 51 = acct */
#else
	"#51 (unimplemented acct)",		/* 51 = unimplemented acct */
#endif
	"#52 (unimplemented sigpending)",		/* 52 = unimplemented sigpending */
	"classcntl",			/* 53 = classcntl */
	"ioctl",			/* 54 = ioctl */
	"reboot",			/* 55 = reboot */
	"revoke",			/* 56 = revoke */
	"symlink",			/* 57 = symlink */
	"readlink",			/* 58 = readlink */
	"execve",			/* 59 = execve */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"#62 (unimplemented old fstat)",		/* 62 = unimplemented old fstat */
	"getpgrp",			/* 63 = getpgrp */
	"getpagesize",			/* 64 = getpagesize */
	"#65 (unimplemented mremap)",		/* 65 = unimplemented mremap */
	"vfork",			/* 66 = vfork */
	"stat",			/* 67 = stat */
	"lstat",			/* 68 = lstat */
	"#69 (unimplemented sbrk)",		/* 69 = unimplemented sbrk */
	"#70 (unimplemented sstk)",		/* 70 = unimplemented sstk */
	"mmap",			/* 71 = mmap */
	"#72 (obsolete vadvise)",		/* 72 = obsolete vadvise */
	"munmap",			/* 73 = munmap */
	"mprotect",			/* 74 = mprotect */
	"madvise",			/* 75 = madvise */
	"#76 (unimplemented old vhangup)",		/* 76 = unimplemented old vhangup */
	"#77 (unimplemented kmodcall)",		/* 77 = unimplemented kmodcall */
	"#78 (unimplemented mincore)",		/* 78 = unimplemented mincore */
	"getgroups",			/* 79 = getgroups */
	"setgroups",			/* 80 = setgroups */
	"#81 (unimplemented old getpgrp)",		/* 81 = unimplemented old getpgrp */
	"setpgrp",			/* 82 = setpgrp */
	"setitimer",			/* 83 = setitimer */
	"#84 (unimplemented old wait)",		/* 84 = unimplemented old wait */
	"#85 (unimplemented table)",		/* 85 = unimplemented table */
	"#86 (unimplemented getitimer)",		/* 86 = unimplemented getitimer */
	"gethostname",			/* 87 = gethostname */
	"sethostname",			/* 88 = sethostname */
	"getdtablesize",			/* 89 = getdtablesize */
	"dup2",			/* 90 = dup2 */
	"fstat",			/* 91 = fstat */
	"fcntl",			/* 92 = fcntl */
	"select",			/* 93 = select */
	"poll",			/* 94 = poll */
	"fsync",			/* 95 = fsync */
	"setpriority",			/* 96 = setpriority */
	"socket",			/* 97 = socket */
	"connect",			/* 98 = connect */
	"accept",			/* 99 = accept */
	"getpriority",			/* 100 = getpriority */
	"send",			/* 101 = send */
	"recv",			/* 102 = recv */
	"sigreturn",			/* 103 = sigreturn */
	"bind",			/* 104 = bind */
	"setsockopt",			/* 105 = setsockopt */
	"listen",			/* 106 = listen */
	"#107 (unimplemented plock)",		/* 107 = unimplemented plock */
	"#108 (unimplemented old sigvec)",		/* 108 = unimplemented old sigvec */
	"#109 (unimplemented old sigblock)",		/* 109 = unimplemented old sigblock */
	"#110 (unimplemented old sigsetmask)",		/* 110 = unimplemented old sigsetmask */
	"sigsuspend",			/* 111 = sigsuspend */
	"sigstack",			/* 112 = sigstack */
	"#113 (unimplemented old recvmsg)",		/* 113 = unimplemented old recvmsg */
	"#114 (unimplemented old sendmsg)",		/* 114 = unimplemented old sendmsg */
	"#115 (obsolete vtrace)",		/* 115 = obsolete vtrace */
	"gettimeofday",			/* 116 = gettimeofday */
	"getrusage",			/* 117 = getrusage */
	"getsockopt",			/* 118 = getsockopt */
	"#119 (unimplemented)",		/* 119 = unimplemented */
	"readv",			/* 120 = readv */
	"writev",			/* 121 = writev */
	"settimeofday",			/* 122 = settimeofday */
	"fchown",			/* 123 = fchown */
	"fchmod",			/* 124 = fchmod */
	"recvfrom",			/* 125 = recvfrom */
	"setreuid",			/* 126 = setreuid */
	"setregid",			/* 127 = setregid */
	"rename",			/* 128 = rename */
	"truncate",			/* 129 = truncate */
	"ftruncate",			/* 130 = ftruncate */
	"#131 (unimplemented flock)",		/* 131 = unimplemented flock */
	"setgid",			/* 132 = setgid */
	"sendto",			/* 133 = sendto */
	"shutdown",			/* 134 = shutdown */
	"socketpair",			/* 135 = socketpair */
	"mkdir",			/* 136 = mkdir */
	"rmdir",			/* 137 = rmdir */
	"utimes",			/* 138 = utimes */
	"#139 (obsolete 4.2 sigreturn)",		/* 139 = obsolete 4.2 sigreturn */
	"#140 (unimplemented adjtime)",		/* 140 = unimplemented adjtime */
	"getpeername",			/* 141 = getpeername */
	"gethostid",			/* 142 = gethostid */
	"sethostid",			/* 143 = sethostid */
	"getrlimit",			/* 144 = getrlimit */
	"setrlimit",			/* 145 = setrlimit */
	"#146 (unimplemented old killpg)",		/* 146 = unimplemented old killpg */
	"setsid",			/* 147 = setsid */
	"#148 (unimplemented quotactl)",		/* 148 = unimplemented quotactl */
	"quota",			/* 149 = quota */
	"getsockname",			/* 150 = getsockname */
	"#151 (unimplemented pread)",		/* 151 = unimplemented pread */
	"#152 (unimplemented pwrite)",		/* 152 = unimplemented pwrite */
	"#153 (unimplemented pid_block)",		/* 153 = unimplemented pid_block */
	"#154 (unimplemented pid_unblock)",		/* 154 = unimplemented pid_unblock */
	"#155 (unimplemented signal_urti)",		/* 155 = unimplemented signal_urti */
	"sigaction",			/* 156 = sigaction */
	"#157 (unimplemented sigwaitprim)",		/* 157 = unimplemented sigwaitprim */
	"#158 (unimplemented nfssvc)",		/* 158 = unimplemented nfssvc */
	"getdirentries",			/* 159 = getdirentries */
	"statfs",			/* 160 = statfs */
	"fstatfs",			/* 161 = fstatfs */
	"#162 (unimplemented)",		/* 162 = unimplemented */
	"#163 (unimplemented async_daemon)",		/* 163 = unimplemented async_daemon */
	"#164 (unimplemented getfh)",		/* 164 = unimplemented getfh */
	"getdomainname",			/* 165 = getdomainname */
	"setdomainname",			/* 166 = setdomainname */
	"#167 (unimplemented)",		/* 167 = unimplemented */
	"#168 (unimplemented)",		/* 168 = unimplemented */
	"#169 (unimplemented exportfs)",		/* 169 = unimplemented exportfs */
	"#170 (unimplemented)",		/* 170 = unimplemented */
	"#171 (unimplemented)",		/* 171 = unimplemented */
	"#172 (unimplemented alt msgctl)",		/* 172 = unimplemented alt msgctl */
	"#173 (unimplemented alt msgget)",		/* 173 = unimplemented alt msgget */
	"#174 (unimplemented alt msgrcv)",		/* 174 = unimplemented alt msgrcv */
	"#175 (unimplemented alt msgsnd)",		/* 175 = unimplemented alt msgsnd */
	"#176 (unimplemented alt semctl)",		/* 176 = unimplemented alt semctl */
	"#177 (unimplemented alt semget)",		/* 177 = unimplemented alt semget */
	"#178 (unimplemented alt semop)",		/* 178 = unimplemented alt semop */
	"#179 (unimplemented alt uname)",		/* 179 = unimplemented alt uname */
	"#180 (unimplemented)",		/* 180 = unimplemented */
	"#181 (unimplemented alt plock)",		/* 181 = unimplemented alt plock */
	"#182 (unimplemented lockf)",		/* 182 = unimplemented lockf */
	"#183 (unimplemented)",		/* 183 = unimplemented */
	"#184 (unimplemented getmnt)",		/* 184 = unimplemented getmnt */
	"#185 (unimplemented)",		/* 185 = unimplemented */
	"#186 (unimplemented unmount)",		/* 186 = unimplemented unmount */
	"#187 (unimplemented alt sigpending)",		/* 187 = unimplemented alt sigpending */
	"#188 (unimplemented alt setsid)",		/* 188 = unimplemented alt setsid */
	"#189 (unimplemented)",		/* 189 = unimplemented */
	"#190 (unimplemented)",		/* 190 = unimplemented */
	"#191 (unimplemented)",		/* 191 = unimplemented */
	"#192 (unimplemented)",		/* 192 = unimplemented */
	"#193 (unimplemented)",		/* 193 = unimplemented */
	"#194 (unimplemented)",		/* 194 = unimplemented */
	"#195 (unimplemented)",		/* 195 = unimplemented */
	"#196 (unimplemented)",		/* 196 = unimplemented */
	"#197 (unimplemented)",		/* 197 = unimplemented */
	"#198 (unimplemented)",		/* 198 = unimplemented */
	"#199 (unimplemented swapon)",		/* 199 = unimplemented swapon */
	"#200 (unimplemented msgctl)",		/* 200 = unimplemented msgctl */
	"#201 (unimplemented msgget)",		/* 201 = unimplemented msgget */
	"#202 (unimplemented msgrcv)",		/* 202 = unimplemented msgrcv */
	"#203 (unimplemented msgsnd)",		/* 203 = unimplemented msgsnd */
	"#204 (unimplemented semctl)",		/* 204 = unimplemented semctl */
	"#205 (unimplemented semget)",		/* 205 = unimplemented semget */
	"#206 (unimplemented semop)",		/* 206 = unimplemented semop */
	"uname",			/* 207 = uname */
	"lchown",			/* 208 = lchown */
	"shmat",			/* 209 = shmat */
	"shmctl",			/* 210 = shmctl */
	"shmdt",			/* 211 = shmdt */
	"shmget",			/* 212 = shmget */
	"#213 (unimplemented mvalid)",		/* 213 = unimplemented mvalid */
	"#214 (unimplemented getaddressconf)",		/* 214 = unimplemented getaddressconf */
	"#215 (unimplemented msleep)",		/* 215 = unimplemented msleep */
	"#216 (unimplemented mwakeup)",		/* 216 = unimplemented mwakeup */
	"#217 (unimplemented msync)",		/* 217 = unimplemented msync */
	"#218 (unimplemented signal)",		/* 218 = unimplemented signal */
	"#219 (unimplemented utc gettime)",		/* 219 = unimplemented utc gettime */
	"#220 (unimplemented utc adjtime)",		/* 220 = unimplemented utc adjtime */
	"#221 (unimplemented)",		/* 221 = unimplemented */
	"#222 (unimplemented security)",		/* 222 = unimplemented security */
	"#223 (unimplemented kloadcall)",		/* 223 = unimplemented kloadcall */
	"#224 (unimplemented)",		/* 224 = unimplemented */
	"#225 (unimplemented)",		/* 225 = unimplemented */
	"#226 (unimplemented)",		/* 226 = unimplemented */
	"#227 (unimplemented)",		/* 227 = unimplemented */
	"#228 (unimplemented)",		/* 228 = unimplemented */
	"#229 (unimplemented)",		/* 229 = unimplemented */
	"#230 (unimplemented)",		/* 230 = unimplemented */
	"#231 (unimplemented)",		/* 231 = unimplemented */
	"#232 (unimplemented)",		/* 232 = unimplemented */
	"#233 (unimplemented getpgid)",		/* 233 = unimplemented getpgid */
	"getsid",			/* 234 = getsid */
	"sigaltstack",			/* 235 = sigaltstack */
	"#236 (unimplemented waitid)",		/* 236 = unimplemented waitid */
	"#237 (unimplemented priocntlset)",		/* 237 = unimplemented priocntlset */
	"#238 (unimplemented sigsendset)",		/* 238 = unimplemented sigsendset */
	"#239 (unimplemented set_speculative)",		/* 239 = unimplemented set_speculative */
	"#240 (unimplemented msfs_syscall)",		/* 240 = unimplemented msfs_syscall */
	"sysinfo",			/* 241 = sysinfo */
	"#242 (unimplemented uadmin)",		/* 242 = unimplemented uadmin */
	"#243 (unimplemented fuser)",		/* 243 = unimplemented fuser */
	"#244 (unimplemented proplist_syscall)",		/* 244 = unimplemented proplist_syscall */
	"#245 (unimplemented ntp_adjtime)",		/* 245 = unimplemented ntp_adjtime */
	"#246 (unimplemented ntp_gettime)",		/* 246 = unimplemented ntp_gettime */
	"pathconf",			/* 247 = pathconf */
	"fpathconf",			/* 248 = fpathconf */
	"#249 (unimplemented)",		/* 249 = unimplemented */
	"#250 (unimplemented uswitch)",		/* 250 = unimplemented uswitch */
	"usleep_thread",			/* 251 = usleep_thread */
	"#252 (unimplemented audcntl)",		/* 252 = unimplemented audcntl */
	"#253 (unimplemented audgen)",		/* 253 = unimplemented audgen */
	"#254 (unimplemented sysfs)",		/* 254 = unimplemented sysfs */
	"#255 (unimplemented subsys_info)",		/* 255 = unimplemented subsys_info */
	"#256 (unimplemented getsysinfo)",		/* 256 = unimplemented getsysinfo */
	"setsysinfo",			/* 257 = setsysinfo */
	"#258 (unimplemented afs_syscall)",		/* 258 = unimplemented afs_syscall */
	"#259 (unimplemented swapctl)",		/* 259 = unimplemented swapctl */
	"#260 (unimplemented memcntl)",		/* 260 = unimplemented memcntl */
	"#261 (unimplemented fdatasync)",		/* 261 = unimplemented fdatasync */
};
