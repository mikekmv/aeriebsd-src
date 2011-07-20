/*
 * Copyright (c) 2011 Michael Shalayeff
 * Copyright (c) 2002 Hiroaki Etoh, Federico G. Schwindt, and Miodrag Vallat.
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD: stack_protector.c,v 1.2 2010/05/14 14:08:10 mickey Exp $";
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/fcntl.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <paths.h>
#include <unistd.h>

extern int __sysctl(int *, u_int, void *, size_t *, void *, size_t);

long __stack_chk_guard[8] = {0, 0, 0, 0, 0, 0, 0, 0};
__weak_alias(__guard, __stack_chk_guard);
static void __guard_setup(void) __attribute__ ((constructor));
void __stack_chk_fail(char func[]);
__weak_alias(__stack_smash_handler, __stack_chk_fail);

static void __attribute__ ((constructor))
__guard_setup(void)
{
	int mib[2];
	size_t len;

	if (__stack_chk_guard[0] != 0)
		return;

	mib[0] = CTL_KERN;
	mib[1] = KERN_ARND;

	len = sizeof(__stack_chk_guard);
	if (__sysctl(mib, 2, __stack_chk_guard, &len, NULL, 0) == -1 ||
	    len != sizeof(__stack_chk_guard)) {
		/* If sysctl was unsuccessful, use the "terminator canary". */
		((unsigned char *)__stack_chk_guard)[0] = 0;
		((unsigned char *)__stack_chk_guard)[1] = 0;
		((unsigned char *)__stack_chk_guard)[2] = '\n';
		((unsigned char *)__stack_chk_guard)[3] = 255;
	}
}

/*ARGSUSED*/
void
__stack_chk_fail(char func[])
{
	extern char *__progname;
	static char pri[] = "<2>";	/* LOG_CRIT */
	static char message[] = ": stack overflow in function ";
	struct sockaddr_un SyslogAddr;	/* AF_UNIX address of local logger */
	struct iovec iov[5], *iop = iov;
	struct sigaction sa;
	sigset_t mask;
	int fd, n = 5;

	/* Immediately block all signal handlers from running code */
	sigfillset(&mask);
	sigdelset(&mask, SIGABRT);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	memset(&SyslogAddr, '\0', sizeof SyslogAddr);
	SyslogAddr.sun_len = sizeof SyslogAddr;
	SyslogAddr.sun_family = AF_UNIX;
	strlcpy(SyslogAddr.sun_path, _PATH_LOG, sizeof SyslogAddr.sun_path);
	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) >= 0) {
		if (connect(fd, (struct sockaddr *)&SyslogAddr,
		    sizeof SyslogAddr)) {
			close(fd);
			fd = -1;
			n = 4;
			iop[3].iov_base = "\r\n";
			iop[3].iov_len = 2;
		} else {
			iop[0].iov_base = pri;
			iop[0].iov_len = sizeof pri - 1;
			iop++;
			iop[3].iov_base = "\n";
			iop[3].iov_len = 1;
		}
	}

	/*
         * Output the message to the console; try not to block
	 * as a blocking console should not stop other processes.
	 */
	if (fd < 0 && (fd = open(_PATH_CONSOLE, O_WRONLY|O_NONBLOCK, 0)) < 0)
		fd = STDERR_FILENO;

	iop[0].iov_base = __progname;
	iop[0].iov_len = strlen(__progname);
	iop[1].iov_base = message;
	iop[1].iov_len = sizeof message - 1;
	iop[2].iov_base = func;
	iop[2].iov_len = strlen(func);
	(void)writev(fd, iov, n);
	(void)close(fd);

	bzero(&sa, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	sigaction(SIGABRT, &sa, NULL);

	kill(getpid(), SIGABRT);

	_exit(127);
}
