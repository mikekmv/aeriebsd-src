
#ifndef __DEFS_H__
#define __DEFS_H__
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $From: defs.h,v 1.6 2001/03/12 18:16:30 kim Exp $
 * @(#)defs.h      5.2 (Berkeley) 3/20/86
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>
#include <syslog.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifndef __GNUC__
# ifndef __attribute__
#  define __attribute__(a)
# endif
#endif

#ifndef __STDC__
# ifndef const
#  define const
# endif
#endif

#include "version.h"
#include "config-def.h"
#include "config.h"
#include "config-data.h"
#include "pathnames.h"
#include "types.h"
#include "filesys.h"

#include <signal.h>

/*
 * This belongs in os-svr4.h but many SVR4 OS's
 * define SVR4 externel to Rdist so we put this
 * check here.
 */
#if	defined(SVR4)
#define NEED_FCNTL_H
#define NEED_NETDB_H
#endif	/* defined(SVR4) */

#if	defined(NEED_NETDB_H)
#include <netdb.h>
#endif	/* NEED_NETDB_H */
#if	defined(NEED_FCNTL_H)
#include <fcntl.h>
#endif	/* NEED_FCNTL_H */
#if	defined(NEED_LIMITS_H)
#include <limits.h>
#endif	/* NEED_LIMITS_H */

#if defined(ARG_TYPE)
#if	ARG_TYPE == ARG_STDARG
#include <stdarg.h>
#endif
#if	ARG_TYPE == ARG_VARARGS
#include <varargs.h>
#endif
#endif	/* ARG_TYPE */

	/* boolean truth */
#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

	/* file modes */
#ifndef S_IXUSR
#define S_IXUSR		0000100
#endif
#ifndef S_IXGRP
#define S_IXGRP		0000010
#endif
#ifndef S_IXOTH
#define S_IXOTH		0000001
#endif

	/* lexical definitions */
#define	QUOTECHAR	160	/* quote next character */

	/* table sizes */
#define HASHSIZE	1021
#define INMAX		3500

	/* expand type definitions */
#define E_VARS		0x1
#define E_SHELL		0x2
#define E_TILDE		0x4
#define E_ALL		0x7

	/* actions for lookup() */
#define LOOKUP		0
#define INSERT		1
#define REPLACE		2

	/* Bit flag test macros */
#define IS_ON(b,f)	(b > 0 && (b & f))
#define IS_OFF(b,f)	!(IS_ON(b,f))
#define FLAG_ON(b,f)	b |= f
#define FLAG_OFF(b,f)	b &= ~(f)

/*
 * POSIX systems should already have S_* defined.
 */
#ifndef S_ISDIR
#define S_ISDIR(m) 	(((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m) 	(((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISLNK
#define S_ISLNK(m) 	(((m) & S_IFMT) == S_IFLNK)
#endif

#define ALLOC(x) 	(struct x *) xmalloc(sizeof(struct x))
#define A(s)		((s) ? s : "<null>")

/*
 * Environment variable names
 */
#define E_FILES		"FILES"			/* List of files */
#define E_LOCFILE	"FILE"			/* Local Filename  */
#define E_REMFILE	"REMFILE"		/* Remote Filename */
#define E_BASEFILE	"BASEFILE"		/* basename of Remote File */

/*
 * Suffix to use when saving files
 */
#ifndef SAVE_SUFFIX
#define SAVE_SUFFIX	".OLD"
#endif

/*
 * Get system error string
 */
#define SYSERR 		strerror(errno)

#define COMMENT_CHAR	'#'		/* Config file comment char */
#define CNULL		'\0'		/* NULL character */

/*
 * These are the top level protocol commands.
 */
#define C_NONE		'='		/* No command - pass cleanly */
#define C_ERRMSG	'\1'		/* Log an error message */
#define C_FERRMSG	'\2'		/* Log a fatal error message */
#define C_NOTEMSG	'\3'		/* Log a note message */
#define C_LOGMSG	'\4'		/* Log a message */
#define C_ACK		'\5'		/* Acknowledge */
#define C_SETCONFIG	'c'		/* Set configuration parameters */
#define C_DIRTARGET	'T'		/* Set target directory name */
#define C_TARGET	't'		/* Set target file name */
#define C_RECVREG	'R'		/* Receive a regular file */
#define C_RECVDIR	'D'		/* Receive a directory */
#define C_RECVSYMLINK	'K'		/* Receive a symbolic link */
#define C_RECVHARDLINK	'k'		/* Receive a hard link */
#define C_END		'E'		/* Indicate end of receive/send */
#define C_CLEAN		'C'		/* Clean up */
#define C_QUERY		'Q'		/* Query without checking */
#define C_SPECIAL	'S'		/* Execute special command */
#define C_CMDSPECIAL	's'		/* Execute cmd special command */
#define C_CHMOG		'M'		/* Chown,Chgrp,Chmod a file */

#define	ack() 		(void) sendcmd(C_ACK, (char *)NULL)
#define	err() 		(void) sendcmd(C_ERRMSG, (char *)NULL)

/*
 * Session startup commands.
 */
#define S_VERSION	'V'		/* Version number */
#define S_REMOTEUSER	'R'		/* Remote user name */
#define S_LOCALUSER	'L'		/* Local user name */
#define S_END		'E'		/* End of session startup commands */

/*
 * These are the commands for "set config".
 */
#define SC_FREESPACE	's'		/* Set min free space */
#define SC_FREEFILES	'f'		/* Set min free files */
#define SC_HOSTNAME	'H'		/* Set client hostname */
#define SC_LOGGING	'L'		/* Set logging options */
#define SC_DEFOWNER	'o'		/* Set default owner */
#define SC_DEFGROUP	'g'		/* Set default group */

/*
 * Query commands
 */
#define QC_ONNFS	'F'		/* File exists & is on a NFS */
#define QC_ONRO		'O'		/* File exists & is on a readonly fs */
#define QC_NO		'N'		/* File does not exist */
#define QC_SYM		'l'		/* File exists & is a symlink */
#define QC_YES		'Y'		/* File does exist */

/*
 * Clean commands
 */
#define CC_QUERY	'Q'		/* Query if file should be rm'ed */
#define CC_END		'E'		/* End of cleaning */
#define CC_YES		'Y'		/* File doesn't exist - remove */
#define CC_NO		'N'		/* File does exist - don't remove */

/*
 * Run Command commands
 */
#define RC_FILE		'F'		/* Name of a target file */
#define RC_COMMAND	'C'		/* Command to run */

/*
 * Name list
 */
struct namelist {		/* for making lists of strings */
	char	*n_name;
	regex_t	*n_regex;
	struct	namelist *n_next;
};

/*
 * Sub command structure
 */
struct subcmd {
	short	sc_type;	/* type - INSTALL,NOTIFY,EXCEPT,SPECIAL */
	opt_t	sc_options;
	char	*sc_name;
	struct	namelist *sc_args;
	struct	subcmd *sc_next;
};

/*
 * Cmd flags
 */
#define CMD_ASSIGNED	0x01	/* This entry has been assigned */
#define CMD_CONNFAILED	0x02	/* Connection failed */
#define CMD_NOCHKNFS	0x04	/* Disable NFS checks */

/*
 * General command structure
 */
struct cmd {
	int	c_type;		/* type - ARROW,DCOLON */
	int	c_flags;	/* flags - CMD_USED,CMD_FAILED */
	char	*c_name;	/* hostname or time stamp file name */
	char	*c_label;	/* label for partial update */
	struct	namelist *c_files;
	struct	subcmd *c_cmds;
	struct	cmd *c_next;
};

/*
 * Hard link buffer information
 */
struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int	count;
	char	*pathname;
	char	*src;
	char	*target;
	struct	linkbuf *nextp;
};

extern char	       *optarg;		/* Option argument */
extern char	       *path_remsh;	/* Remote shell command */
extern char 		host[];		/* Host name of master copy */
extern char 	       *currenthost;	/* Name of current host */
extern char 	       *progname;	/* Name of this program */
extern char 	      **realargv;	/* Real argv */
extern int		optind;		/* Option index into argv */
extern int 		debug;		/* Debugging flag */
extern opt_t 		defoptions;	/* Default install options */
extern int 		do_fork;	/* Should we do fork()'ing */
#ifndef __STDC__
extern int 		errno;		/* System error number */
#endif
extern int 		isserver;	/* Acting as remote server */
extern int 		nerrs;		/* Number of errors seen */
extern int 		nflag;		/* NOP flag, don't execute commands */
extern opt_t 		options;	/* Global options */
extern int 		proto_version;	/* Protocol version number */
extern int 		realargc;	/* Real argc */
extern int		rem_r;		/* Remote file descriptor, reading */
extern int 		rem_w;		/* Remote file descriptor, writing */
extern int 		rtimeout;	/* Response time out in seconds */
extern int		setjmp_ok;	/* setjmp/longjmp flag */
extern UID_T 		userid;		/* User ID of rdist user */
extern jmp_buf 		finish_jmpbuf;	/* Setjmp buffer for finish() */
extern struct group    *gr;	/* pointer to static area used by getgrent */
extern struct linkbuf  *ihead;	/* list of files with more than one link */
extern struct passwd   *pw;	/* pointer to static area used by getpwent */
extern char defowner[64];		/* Default owner */
extern char defgroup[64];		/* Default group */
extern volatile sig_atomic_t contimedout; /* Connection timed out */
#ifdef USE_STATDB
extern int 		dostatdb;
extern int 		juststatdb;
#endif /* USE_STATDB */

/*
 * Our own declarations.
 */

/* child.c */
void waitup(void);
int spawn(struct cmd *, struct cmd *);
int setnonblocking(int, int);
int setnonblocking(int, int);

/* client.c */
char *remfilename(char *, char *, char *, char *, int);
int inlist(struct namelist *, char *);
void runcmdspecial(struct cmd *, opt_t);
int checkfilename(char *);
void freelinkinfo(struct linkbuf *);
void cleanup(int);
int install(char *, char *, int, int , opt_t);

/* common.c */
WRITE_RETURN_T xwrite(int, void *, WRITE_AMT_T);
int init(int, char **, char **);
void finish(void);
void lostconn(void);
void coredump(void);
void sighandler(int);
int sendcmd(char, char *, ...);
int remline(u_char *, int, int);
int readrem(char *, int);
char *getusername(UID_T, char *, opt_t);
char *getgroupname(GID_T, char *, opt_t);
int response(void);
char *exptilde(char *, char *, size_t);
int becomeuser(void);
int becomeroot(void);
int setfiletime(char *, time_t, time_t);
char *getversion(void);
void runcommand(char *);
char *xmalloc(size_t);
char *xrealloc(char *, size_t);
char *xcalloc(size_t, size_t);
char *xstrdup(const char *);
char *xbasename(char *);
char *searchpath(char *);
int mysetlinebuf(FILE *);

/* distopt.c */
DISTOPTINFO *getdistopt(char *, int *);
int parsedistopts(char *, opt_t *, int);
char *getdistoptlist(void);
char *getondistoptlist(opt_t);

/* docmd.c */
void markassigned(struct cmd *, struct cmd *);
int okname(char *);
int except(char *);
void docmds(struct namelist *, int, char **);

/* expand.c */
struct namelist *expand(struct namelist *, int);
u_char *xstrchr(u_char *, int);
void expstr(u_char *);
void expsh(u_char *);
void matchdir(char *);
int execbrc(u_char *, u_char *);
int match(char *, char *);
int amatch(char *, u_char *);

/* filesys.c */
char *find_file(char *, struct stat *, int *);
mntent_t *findmnt(struct stat *, struct mntinfo *);
int isdupmnt(mntent_t *, struct mntinfo *);
void wakeup(int);
struct mntinfo *makemntinfo(struct mntinfo *);
mntent_t *getmntpt(char *, struct stat *, int *);
int is_nfs_mounted(char *, struct stat *, int *);
int is_ro_mounted(char *, struct stat *, int *);
int is_symlinked(char *, struct stat *, int *);
int getfilesysinfo(char *, long *, long *);

/* gram.c */
int yylex(void);
int any(int, char *);
void insert(char *, struct namelist *, struct namelist *, struct subcmd *);
void append(char *, struct namelist *, char *, struct subcmd *);
void yyerror(char *);
struct namelist *makenl(char *);
struct subcmd *makesubcmd(int);
int yyparse(void);

/* hasmntopt.c */
#ifdef NEED_HASMNTOPT
char *hasmntopt(struct mntent *, char *);
#endif

/* isexec.c */
int isexec(char *, struct stat *);

/* lookup.c */
void define(char *);
struct namelist *lookup(char *, int, struct namelist *);

/* message.c */
void msgprusage(void);
void msgprconfig(void);
char *msgparseopts(char *, int);
void checkhostname(void);
void message(int, char *, ...);
void debugmsg(int, char *, ...);
void error(char *, ...);
void fatalerr(char *, ...);
char *getnotifyfile(void);

/* rdist.c */
FILE *opendist(char *);
void docmdargs(int, char *[]);
char *getnlstr(struct namelist *);

/* rshrcmd.c */
int rshrcmd(char **, u_short, char *, char *, char *, int *);

/* server.c */
void server(void);

/* setargs.c */
void setargs_settup(int, char **, char **);
void _setproctitle(char *);
void setproctitle(const char *, ...);

/* signal.c */
#ifdef NEED_SIGBLOCK
int sigblock(int);
#endif
#ifdef NEED_SIGMASK
int sigsetmask(int);
#endif

/* strcasecmp.c */
#ifdef NEED_STRCASECMP
int strcasecmp(char *, char *);
int strncasecmp(char *, char *, int);
#endif

/* strerror.c */
#ifdef NEED_STRERROR
char *strerror(int);
#endif

/* strtol.c */
#ifdef NEED_STRTOL
long strtol(char *, char **, int);
#endif

/* unvis.c */
#ifdef NEED_VIS
int unvis(char *, int, int *, int );
int strunvis(char *, const char *);
#endif

/* vis.c */
#ifdef NEED_VIS
char *vis(char *, int, int, int );
int strvis(char *, const char *, int);
int strvisx(char *, const char *, size_t, int);
#endif

/* vsnprintf.c */
#ifdef NEED_VSNPRINTF
int vsnprintf(char *, size_t, const char *, va_list);
int snprintf(char *, size_t, const char *, ...);
#endif

/* zopen.c */
int zread(void *, char *, int);
int zwrite(void *, const char *, int);
int z_close(void *);
void *z_open(int, char *, int);

#include <vis.h>
#define DECODE(a, b)	strunvis(a, b)
#define ENCODE(a, b)	strvis(a, b, VIS_WHITE)

#endif	/* __DEFS_H__ */
