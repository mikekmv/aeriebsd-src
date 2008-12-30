/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 *
 * created from
 *	
 */

char *syscallnames[] = {
	"syscall",			/* 0 = syscall */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"open",			/* 5 = open */
	"close",			/* 6 = close */
	"wait4",			/* 7 = wait4 */
	"compat_43_ocreat",	/* 8 = compat_43 ocreat */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"#11 (obsolete execv)",		/* 11 = obsolete execv */
	"chdir",			/* 12 = chdir */
	"fchdir",			/* 13 = fchdir */
	"mknod",			/* 14 = mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown */
	"break",			/* 17 = break */
	"compat_25_ogetfsstat",	/* 18 = compat_25 ogetfsstat */
	"compat_43_olseek",	/* 19 = compat_43 olseek */
	"getpid",			/* 20 = getpid */
	"mount",			/* 21 = mount */
	"unmount",			/* 22 = unmount */
	"setuid",			/* 23 = setuid */
	"getuid",			/* 24 = getuid */
	"geteuid",			/* 25 = geteuid */
#ifdef PTRACE
	"ptrace",			/* 26 = ptrace */
#else
	"#26 (unimplemented ptrace)",		/* 26 = unimplemented ptrace */
#endif
	"recvmsg",			/* 27 = recvmsg */
	"sendmsg",			/* 28 = sendmsg */
	"recvfrom",			/* 29 = recvfrom */
	"accept",			/* 30 = accept */
	"getpeername",			/* 31 = getpeername */
	"getsockname",			/* 32 = getsockname */
	"access",			/* 33 = access */
	"chflags",			/* 34 = chflags */
	"fchflags",			/* 35 = fchflags */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"compat_43_stat43",	/* 38 = compat_43 stat43 */
	"getppid",			/* 39 = getppid */
	"compat_43_lstat43",	/* 40 = compat_43 lstat43 */
	"dup",			/* 41 = dup */
	"opipe",			/* 42 = opipe */
	"getegid",			/* 43 = getegid */
	"profil",			/* 44 = profil */
#ifdef KTRACE
	"ktrace",			/* 45 = ktrace */
#else
	"#45 (unimplemented ktrace)",		/* 45 = unimplemented ktrace */
#endif
	"sigaction",			/* 46 = sigaction */
	"getgid",			/* 47 = getgid */
	"sigprocmask",			/* 48 = sigprocmask */
	"getlogin",			/* 49 = getlogin */
	"setlogin",			/* 50 = setlogin */
#ifdef ACCOUNTING
	"acct",			/* 51 = acct */
#else
	"#51 (unimplemented acct)",		/* 51 = unimplemented acct */
#endif
	"sigpending",			/* 52 = sigpending */
	"osigaltstack",			/* 53 = osigaltstack */
	"ioctl",			/* 54 = ioctl */
	"reboot",			/* 55 = reboot */
	"revoke",			/* 56 = revoke */
	"symlink",			/* 57 = symlink */
	"readlink",			/* 58 = readlink */
	"execve",			/* 59 = execve */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"compat_43_fstat43",	/* 62 = compat_43 fstat43 */
	"compat_43_ogetkerninfo",	/* 63 = compat_43 ogetkerninfo */
	"compat_43_ogetpagesize",	/* 64 = compat_43 ogetpagesize */
	"compat_25_omsync",	/* 65 = compat_25 omsync */
	"vfork",			/* 66 = vfork */
	"#67 (obsolete vread)",		/* 67 = obsolete vread */
	"#68 (obsolete vwrite)",		/* 68 = obsolete vwrite */
	"sbrk",			/* 69 = sbrk */
	"sstk",			/* 70 = sstk */
	"compat_43_ommap",	/* 71 = compat_43 ommap */
	"#72 (obsolete vadvise)",		/* 72 = obsolete vadvise */
	"munmap",			/* 73 = munmap */
	"mprotect",			/* 74 = mprotect */
	"madvise",			/* 75 = madvise */
	"#76 (obsolete vhangup)",		/* 76 = obsolete vhangup */
	"#77 (obsolete vlimit)",		/* 77 = obsolete vlimit */
	"mincore",			/* 78 = mincore */
	"getgroups",			/* 79 = getgroups */
	"setgroups",			/* 80 = setgroups */
	"getpgrp",			/* 81 = getpgrp */
	"setpgid",			/* 82 = setpgid */
	"setitimer",			/* 83 = setitimer */
	"compat_43_owait",	/* 84 = compat_43 owait */
	"compat_25_swapon",	/* 85 = compat_25 swapon */
	"getitimer",			/* 86 = getitimer */
	"compat_43_ogethostname",	/* 87 = compat_43 ogethostname */
	"compat_43_osethostname",	/* 88 = compat_43 osethostname */
	"compat_43_ogetdtablesize",	/* 89 = compat_43 ogetdtablesize */
	"dup2",			/* 90 = dup2 */
	"#91 (unimplemented getdopt)",		/* 91 = unimplemented getdopt */
	"fcntl",			/* 92 = fcntl */
	"select",			/* 93 = select */
	"#94 (unimplemented setdopt)",		/* 94 = unimplemented setdopt */
	"fsync",			/* 95 = fsync */
	"setpriority",			/* 96 = setpriority */
	"socket",			/* 97 = socket */
	"connect",			/* 98 = connect */
	"compat_43_oaccept",	/* 99 = compat_43 oaccept */
	"getpriority",			/* 100 = getpriority */
	"compat_43_osend",	/* 101 = compat_43 osend */
	"compat_43_orecv",	/* 102 = compat_43 orecv */
	"sigreturn",			/* 103 = sigreturn */
	"bind",			/* 104 = bind */
	"setsockopt",			/* 105 = setsockopt */
	"listen",			/* 106 = listen */
	"#107 (obsolete vtimes)",		/* 107 = obsolete vtimes */
	"compat_43_osigvec",	/* 108 = compat_43 osigvec */
	"compat_43_osigblock",	/* 109 = compat_43 osigblock */
	"compat_43_osigsetmask",	/* 110 = compat_43 osigsetmask */
	"sigsuspend",			/* 111 = sigsuspend */
	"compat_43_osigstack",	/* 112 = compat_43 osigstack */
	"compat_43_orecvmsg",	/* 113 = compat_43 orecvmsg */
	"compat_43_osendmsg",	/* 114 = compat_43 osendmsg */
	"#115 (obsolete vtrace)",		/* 115 = obsolete vtrace */
	"gettimeofday",			/* 116 = gettimeofday */
	"getrusage",			/* 117 = getrusage */
	"getsockopt",			/* 118 = getsockopt */
	"#119 (obsolete resuba)",		/* 119 = obsolete resuba */
	"readv",			/* 120 = readv */
	"writev",			/* 121 = writev */
	"settimeofday",			/* 122 = settimeofday */
	"fchown",			/* 123 = fchown */
	"fchmod",			/* 124 = fchmod */
	"compat_43_orecvfrom",	/* 125 = compat_43 orecvfrom */
	"setreuid",			/* 126 = setreuid */
	"setregid",			/* 127 = setregid */
	"rename",			/* 128 = rename */
	"compat_43_otruncate",	/* 129 = compat_43 otruncate */
	"compat_43_oftruncate",	/* 130 = compat_43 oftruncate */
	"flock",			/* 131 = flock */
	"mkfifo",			/* 132 = mkfifo */
	"sendto",			/* 133 = sendto */
	"shutdown",			/* 134 = shutdown */
	"socketpair",			/* 135 = socketpair */
	"mkdir",			/* 136 = mkdir */
	"rmdir",			/* 137 = rmdir */
	"utimes",			/* 138 = utimes */
	"#139 (obsolete 4.2 sigreturn)",		/* 139 = obsolete 4.2 sigreturn */
	"adjtime",			/* 140 = adjtime */
	"compat_43_ogetpeername",	/* 141 = compat_43 ogetpeername */
	"compat_43_ogethostid",	/* 142 = compat_43 ogethostid */
	"compat_43_osethostid",	/* 143 = compat_43 osethostid */
	"compat_43_ogetrlimit",	/* 144 = compat_43 ogetrlimit */
	"compat_43_osetrlimit",	/* 145 = compat_43 osetrlimit */
	"compat_43_okillpg",	/* 146 = compat_43 okillpg */
	"setsid",			/* 147 = setsid */
	"quotactl",			/* 148 = quotactl */
	"compat_43_oquota",	/* 149 = compat_43 oquota */
	"compat_43_ogetsockname",	/* 150 = compat_43 ogetsockname */
	"#151 (unimplemented)",		/* 151 = unimplemented */
	"#152 (unimplemented)",		/* 152 = unimplemented */
	"#153 (unimplemented)",		/* 153 = unimplemented */
	"#154 (unimplemented)",		/* 154 = unimplemented */
#if defined(NFSCLIENT) || defined(NFSSERVER)
	"nfssvc",			/* 155 = nfssvc */
#else
	"#155 (unimplemented)",		/* 155 = unimplemented */
#endif
	"compat_43_ogetdirentries",	/* 156 = compat_43 ogetdirentries */
	"compat_25_ostatfs",	/* 157 = compat_25 ostatfs */
	"compat_25_ostatfs",	/* 158 = compat_25 ostatfs */
	"#159 (unimplemented)",		/* 159 = unimplemented */
	"#160 (unimplemented)",		/* 160 = unimplemented */
	"getfh",			/* 161 = getfh */
	"compat_09_ogetdomainname",	/* 162 = compat_09 ogetdomainname */
	"compat_09_osetdomainname",	/* 163 = compat_09 osetdomainname */
	"#164 (unimplemented ouname)",		/* 164 = unimplemented ouname */
	"sysarch",			/* 165 = sysarch */
	"#166 (unimplemented)",		/* 166 = unimplemented */
	"#167 (unimplemented)",		/* 167 = unimplemented */
	"#168 (unimplemented)",		/* 168 = unimplemented */
#if defined(SYSVSEM) && !defined(__LP64__)
	"compat_10_osemsys",	/* 169 = compat_10 osemsys */
#else
	"#169 (unimplemented 1.0 semsys)",		/* 169 = unimplemented 1.0 semsys */
#endif
#if defined(SYSVMSG) && !defined(__LP64__)
	"compat_10_omsgsys",	/* 170 = compat_10 omsgsys */
#else
	"#170 (unimplemented 1.0 msgsys)",		/* 170 = unimplemented 1.0 msgsys */
#endif
#if defined(SYSVSHM) && !defined(__LP64__)
	"compat_10_oshmsys",	/* 171 = compat_10 oshmsys */
#else
	"#171 (unimplemented 1.0 shmsys)",		/* 171 = unimplemented 1.0 shmsys */
#endif
	"#172 (unimplemented)",		/* 172 = unimplemented */
	"pread",			/* 173 = pread */
	"pwrite",			/* 174 = pwrite */
	"#175 (unimplemented ntp_gettime)",		/* 175 = unimplemented ntp_gettime */
	"#176 (unimplemented ntp_adjtime)",		/* 176 = unimplemented ntp_adjtime */
	"#177 (unimplemented)",		/* 177 = unimplemented */
	"#178 (unimplemented)",		/* 178 = unimplemented */
	"#179 (unimplemented)",		/* 179 = unimplemented */
	"#180 (unimplemented)",		/* 180 = unimplemented */
	"setgid",			/* 181 = setgid */
	"setegid",			/* 182 = setegid */
	"seteuid",			/* 183 = seteuid */
#ifdef LFS
	"lfs_bmapv",			/* 184 = lfs_bmapv */
	"lfs_markv",			/* 185 = lfs_markv */
	"lfs_segclean",			/* 186 = lfs_segclean */
	"lfs_segwait",			/* 187 = lfs_segwait */
#else
	"#184 (unimplemented)",		/* 184 = unimplemented */
	"#185 (unimplemented)",		/* 185 = unimplemented */
	"#186 (unimplemented)",		/* 186 = unimplemented */
	"#187 (unimplemented)",		/* 187 = unimplemented */
#endif
	"compat_35_stat35",	/* 188 = compat_35 stat35 */
	"compat_35_fstat35",	/* 189 = compat_35 fstat35 */
	"compat_35_lstat35",	/* 190 = compat_35 lstat35 */
	"pathconf",			/* 191 = pathconf */
	"fpathconf",			/* 192 = fpathconf */
	"swapctl",			/* 193 = swapctl */
	"getrlimit",			/* 194 = getrlimit */
	"setrlimit",			/* 195 = setrlimit */
	"getdirentries",			/* 196 = getdirentries */
	"mmap",			/* 197 = mmap */
	"__syscall",			/* 198 = __syscall */
	"lseek",			/* 199 = lseek */
	"truncate",			/* 200 = truncate */
	"ftruncate",			/* 201 = ftruncate */
	"__sysctl",			/* 202 = __sysctl */
	"mlock",			/* 203 = mlock */
	"munlock",			/* 204 = munlock */
	"#205 (unimplemented sys_undelete)",		/* 205 = unimplemented sys_undelete */
	"futimes",			/* 206 = futimes */
	"getpgid",			/* 207 = getpgid */
	"xfspioctl",			/* 208 = xfspioctl */
	"#209 (unimplemented)",		/* 209 = unimplemented */
#ifdef LKM
	"lkmnosys",			/* 210 = lkmnosys */
	"lkmnosys",			/* 211 = lkmnosys */
	"lkmnosys",			/* 212 = lkmnosys */
	"lkmnosys",			/* 213 = lkmnosys */
	"lkmnosys",			/* 214 = lkmnosys */
	"lkmnosys",			/* 215 = lkmnosys */
	"lkmnosys",			/* 216 = lkmnosys */
	"lkmnosys",			/* 217 = lkmnosys */
	"lkmnosys",			/* 218 = lkmnosys */
	"lkmnosys",			/* 219 = lkmnosys */
#else	/* !LKM */
	"#210 (unimplemented)",		/* 210 = unimplemented */
	"#211 (unimplemented)",		/* 211 = unimplemented */
	"#212 (unimplemented)",		/* 212 = unimplemented */
	"#213 (unimplemented)",		/* 213 = unimplemented */
	"#214 (unimplemented)",		/* 214 = unimplemented */
	"#215 (unimplemented)",		/* 215 = unimplemented */
	"#216 (unimplemented)",		/* 216 = unimplemented */
	"#217 (unimplemented)",		/* 217 = unimplemented */
	"#218 (unimplemented)",		/* 218 = unimplemented */
	"#219 (unimplemented)",		/* 219 = unimplemented */
#endif	/* !LKM */
#ifdef SYSVSEM
	"compat_23_semctl23",	/* 220 = compat_23 semctl23 */
	"semget",			/* 221 = semget */
	"compat_35_semop",	/* 222 = compat_35 semop */
	"#223 (obsolete sys_semconfig)",		/* 223 = obsolete sys_semconfig */
#else
	"#220 (unimplemented semctl)",		/* 220 = unimplemented semctl */
	"#221 (unimplemented semget)",		/* 221 = unimplemented semget */
	"#222 (unimplemented semop)",		/* 222 = unimplemented semop */
	"#223 (unimplemented semconfig)",		/* 223 = unimplemented semconfig */
#endif
#ifdef SYSVMSG
	"compat_23_msgctl23",	/* 224 = compat_23 msgctl23 */
	"msgget",			/* 225 = msgget */
	"msgsnd",			/* 226 = msgsnd */
	"msgrcv",			/* 227 = msgrcv */
#else
	"#224 (unimplemented msgctl)",		/* 224 = unimplemented msgctl */
	"#225 (unimplemented msgget)",		/* 225 = unimplemented msgget */
	"#226 (unimplemented msgsnd)",		/* 226 = unimplemented msgsnd */
	"#227 (unimplemented msgrcv)",		/* 227 = unimplemented msgrcv */
#endif
#ifdef SYSVSHM
	"shmat",			/* 228 = shmat */
	"compat_23_shmctl23",	/* 229 = compat_23 shmctl23 */
	"shmdt",			/* 230 = shmdt */
	"compat_35_shmget",	/* 231 = compat_35 shmget */
#else
	"#228 (unimplemented shmat)",		/* 228 = unimplemented shmat */
	"#229 (unimplemented shmctl)",		/* 229 = unimplemented shmctl */
	"#230 (unimplemented shmdt)",		/* 230 = unimplemented shmdt */
	"#231 (unimplemented shmget)",		/* 231 = unimplemented shmget */
#endif
	"clock_gettime",			/* 232 = clock_gettime */
	"clock_settime",			/* 233 = clock_settime */
	"clock_getres",			/* 234 = clock_getres */
	"#235 (unimplemented timer_create)",		/* 235 = unimplemented timer_create */
	"#236 (unimplemented timer_delete)",		/* 236 = unimplemented timer_delete */
	"#237 (unimplemented timer_settime)",		/* 237 = unimplemented timer_settime */
	"#238 (unimplemented timer_gettime)",		/* 238 = unimplemented timer_gettime */
	"#239 (unimplemented timer_getoverrun)",		/* 239 = unimplemented timer_getoverrun */
	"nanosleep",			/* 240 = nanosleep */
	"#241 (unimplemented)",		/* 241 = unimplemented */
	"#242 (unimplemented)",		/* 242 = unimplemented */
	"#243 (unimplemented)",		/* 243 = unimplemented */
	"#244 (unimplemented)",		/* 244 = unimplemented */
	"#245 (unimplemented)",		/* 245 = unimplemented */
	"#246 (unimplemented)",		/* 246 = unimplemented */
	"#247 (unimplemented)",		/* 247 = unimplemented */
	"#248 (unimplemented)",		/* 248 = unimplemented */
	"#249 (unimplemented)",		/* 249 = unimplemented */
	"minherit",			/* 250 = minherit */
	"rfork",			/* 251 = rfork */
	"poll",			/* 252 = poll */
	"issetugid",			/* 253 = issetugid */
	"lchown",			/* 254 = lchown */
	"getsid",			/* 255 = getsid */
	"msync",			/* 256 = msync */
#ifdef SYSVSEM
	"compat_35_semctl35",	/* 257 = compat_35 semctl35 */
#else
	"#257 (unimplemented)",		/* 257 = unimplemented */
#endif
#ifdef SYSVSHM
	"compat_35_shmctl35",	/* 258 = compat_35 shmctl35 */
#else
	"#258 (unimplemented)",		/* 258 = unimplemented */
#endif
#ifdef SYSVMSG
	"compat_35_msgctl35",	/* 259 = compat_35 msgctl35 */
#else
	"#259 (unimplemented)",		/* 259 = unimplemented */
#endif
	"compat_o43_getfsstat",	/* 260 = compat_o43 getfsstat */
	"compat_o43_statfs",	/* 261 = compat_o43 statfs */
	"compat_o43_fstatfs",	/* 262 = compat_o43 fstatfs */
	"pipe",			/* 263 = pipe */
	"fhopen",			/* 264 = fhopen */
	"compat_35_fhstat",	/* 265 = compat_35 fhstat */
	"compat_o43_fhstatfs",	/* 266 = compat_o43 fhstatfs */
	"preadv",			/* 267 = preadv */
	"pwritev",			/* 268 = pwritev */
	"kqueue",			/* 269 = kqueue */
	"kevent",			/* 270 = kevent */
	"mlockall",			/* 271 = mlockall */
	"munlockall",			/* 272 = munlockall */
	"getpeereid",			/* 273 = getpeereid */
	"#274 (unimplemented sys_extattrctl)",		/* 274 = unimplemented sys_extattrctl */
	"#275 (unimplemented sys_extattr_set_file)",		/* 275 = unimplemented sys_extattr_set_file */
	"#276 (unimplemented sys_extattr_get_file)",		/* 276 = unimplemented sys_extattr_get_file */
	"#277 (unimplemented sys_extattr_delete_file)",		/* 277 = unimplemented sys_extattr_delete_file */
	"#278 (unimplemented sys_extattr_set_fd)",		/* 278 = unimplemented sys_extattr_set_fd */
	"#279 (unimplemented sys_extattr_get_fd)",		/* 279 = unimplemented sys_extattr_get_fd */
	"#280 (unimplemented sys_extattr_delete_fd)",		/* 280 = unimplemented sys_extattr_delete_fd */
	"getresuid",			/* 281 = getresuid */
	"setresuid",			/* 282 = setresuid */
	"getresgid",			/* 283 = getresgid */
	"setresgid",			/* 284 = setresgid */
	"#285 (obsolete sys_omquery)",		/* 285 = obsolete sys_omquery */
	"mquery",			/* 286 = mquery */
	"closefrom",			/* 287 = closefrom */
	"sigaltstack",			/* 288 = sigaltstack */
#ifdef SYSVSHM
	"shmget",			/* 289 = shmget */
#else
	"#289 (unimplemented shmget)",		/* 289 = unimplemented shmget */
#endif
#ifdef SYSVSEM
	"semop",			/* 290 = semop */
#else
	"#290 (unimplemented semop)",		/* 290 = unimplemented semop */
#endif
	"stat",			/* 291 = stat */
	"fstat",			/* 292 = fstat */
	"lstat",			/* 293 = lstat */
	"fhstat",			/* 294 = fhstat */
#ifdef SYSVSEM
	"__semctl",			/* 295 = __semctl */
#else
	"#295 (unimplemented)",		/* 295 = unimplemented */
#endif
#ifdef SYSVSHM
	"shmctl",			/* 296 = shmctl */
#else
	"#296 (unimplemented)",		/* 296 = unimplemented */
#endif
#ifdef SYSVMSG
	"msgctl",			/* 297 = msgctl */
#else
	"#297 (unimplemented)",		/* 297 = unimplemented */
#endif
	"sched_yield",			/* 298 = sched_yield */
#ifdef RTHREADS
	"getthrid",			/* 299 = getthrid */
	"thrsleep",			/* 300 = thrsleep */
	"thrwakeup",			/* 301 = thrwakeup */
	"threxit",			/* 302 = threxit */
	"thrsigdivert",			/* 303 = thrsigdivert */
#else
	"#299 (unimplemented)",		/* 299 = unimplemented */
	"#300 (unimplemented)",		/* 300 = unimplemented */
	"#301 (unimplemented)",		/* 301 = unimplemented */
	"#302 (unimplemented)",		/* 302 = unimplemented */
	"#303 (unimplemented)",		/* 303 = unimplemented */
#endif
	"__getcwd",			/* 304 = __getcwd */
	"adjfreq",			/* 305 = adjfreq */
	"getfsstat",			/* 306 = getfsstat */
	"statfs",			/* 307 = statfs */
	"fstatfs",			/* 308 = fstatfs */
	"fhstatfs",			/* 309 = fhstatfs */
};