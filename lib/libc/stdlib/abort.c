/*
 * Copyright (c) 1985 Regents of the University of California.
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

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$ABSD$";
#endif

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "thread_private.h"
#include "atexit.h"

void
abort(void)
{
	struct atexit *p = __atexit;
	static int cleanup_called = 0;
	sigset_t mask;


	sigfillset(&mask);
	/*
	 * don't block SIGABRT to give any handler a chance; we ignore
	 * any errors -- X311J doesn't allow abort to return anyway.
	 */
	sigdelset(&mask, SIGABRT);
	(void)_thread_sys_sigprocmask(SIG_SETMASK, &mask, (sigset_t *)NULL);

	/*
	 * POSIX requires we flush stdio buffers on abort
	 */
	if (cleanup_called == 0) {
		/* the cleanup routine lives in fns[0] on the last page */
		while (p != NULL && p->next != NULL)
			p = p->next;
		/* the check for fn_dso == NULL is mostly paranoia */
		if (p != NULL && p->fns[0].fn_dso == NULL &&
		    p->fns[0].fn_ptr.std_func != NULL) {
			cleanup_called = 1;
			(*p->fns[0].fn_ptr.std_func)();
		}
	}

	(void)kill(getpid(), SIGABRT);

	/*
	 * if SIGABRT ignored, or caught and the handler returns, do
	 * it again, only harder.
	 */
	(void)signal(SIGABRT, SIG_DFL);
	(void)_thread_sys_sigprocmask(SIG_SETMASK, &mask, (sigset_t *)NULL);
	(void)kill(getpid(), SIGABRT);
	_exit(1);
}
