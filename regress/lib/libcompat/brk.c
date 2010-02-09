/*
 * Copyright (c) 2010 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include <err.h>

static void
sigsegv(int sig, siginfo_t *sip, void *scp)
{
	_exit(0);
}

int
main()
{
	extern char end;
	struct sigaction sa;
	char *p, *q;

	p = sbrk(0);
	if (p < &end)
		errx(1, "The End is NEAR!");

	if (!sbrk(10))
		err(1, "sbrk(10)");

	if (!(q = sbrk(20)))
		err(2, "sbrk(20)");

	if (q - p < 10)
		errx(2, "grow is too low (%ld < %d)", q - p, 10);

	/* do some serious growth */
	if (!(p = sbrk(30000)))
		err(3, "sbrk(30000)");
	memset(p, 0, 30000);

	/* provide the shrinkage */
	if (brk(p - 1) < 0)
		err(4, "brk");

	sa.sa_sigaction = &sigsegv;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
	memset(p, 0, 30000);

	return 5;
}
