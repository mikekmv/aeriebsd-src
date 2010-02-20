/*
 * Copyright (c) 2009 Mike Belopuhov
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define ICB_MSGSIZE		 255

#define ICB_MAXGRPLEN		 32
#define ICB_MAXNICKLEN		 32
#define ICB_MAXPASSLEN		 32
#define ICB_MAXTOPICLEN		 160

#define ICB_M_LOGIN		 'a'
#define ICB_M_OPEN		 'b'
#define ICB_M_PERSONAL		 'c'
#define ICB_M_STATUS		 'd'
enum {
	 STATUS_AWAY,
	 STATUS_ARRIVE,
	 STATUS_BOOT,
	 STATUS_DEPART,
	 STATUS_NOAWAY,
	 STATUS_NOTIFY,
	 STATUS_SIGNON,
	 STATUS_SIGNOFF,
	 STATUS_STATUS,
	 STATUS_TOPIC,
	 STATUS_WARNING
};
#define ICB_M_ERROR		 'e'
#define ICB_M_IMPORTANT		 'f'
#define ICB_M_EXIT		 'g'
#define ICB_M_COMMAND		 'h'
enum {
	 CMD_NOTCMD,
	 CMD_ICBCMD,
	 CMD_HELP,
	 CMD_QUIT
};
#define ICB_M_CMDOUT		 'i'
enum {
	 CMDOUT_CO,
	 CMDOUT_EC,
	 CMDOUT_WL,
	 CMDOUT_WG,
};
#define ICB_M_PROTO		 'j'
#define ICB_M_BEEP		 'k'
#define ICB_M_PING		 'l'
#define ICB_M_PONG		 'm'
#define ICB_M_NOOP		 'n'

#define ICB_M_SEP		 '\001'

#define ICB_MAXFIELDS		 10
