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

#ifndef __ICB_H__
#define __ICB_H__

#define ICB_MSGSIZE		 256

#define ICB_MAXGRPLEN		 32
#define ICB_MAXNICKLEN		 32
#define ICB_MAXPASSLEN		 32

#define ICB_LOG_ERROR		 0
#define ICB_LOG_NORMAL		 1
#define ICB_LOG_DEBUG		 2

#define ICB_M_LOGIN		 'a'
#define ICB_M_OPEN		 'b'
#define ICB_M_PERSONAL		 'c'
#define ICB_M_STATUS		 'd'
enum {
	 STATUS_AWAY,
	 STATUS_ARRIVE,
	 STATUS_BOOT,
	 STATUS_DEPART,
	 STATUS_NOTIFY,
	 STATUS_SIGNON,
	 STATUS_SIGNOFF,
	 STATUS_STATUS,
	 STATUS_WARNING
};
#define ICB_M_ERROR		 'e'
#define ICB_M_IMPORTANT		 'f'
#define ICB_M_EXIT		 'g'
#define ICB_M_COMMAND		 'h'
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

struct icb_group;

struct icb_session {
	LIST_ENTRY(icb_session)	 entry;
	void			*upper;
	struct icb_group	*group;
	char			 nick[ICB_MAXNICKLEN];
	char			 client[ICB_MAXNICKLEN];
	char			 host[MAXHOSTNAMELEN];
	uint32_t		 flags;
#define SETF(t, f)		 ((t) |= (f))
#define CLRF(t, f)		 ((t) &= ~(f))
#define ISSETF(t, f)		 ((t) & (f))
#define ICB_SF_UNKNOWN		 0x00
#define ICB_SF_PROTOSENT	 0x01
#define ICB_SF_LOGGEDIN		 0x02
#define ICB_SF_NOGROUP		 0x08
#define ICB_SF_AWAY		 0x10
#define ICB_SF_MODERATOR	 0x20
	time_t			 login;
	time_t			 last;
};

struct icb_group {
	LIST_ENTRY(icb_group)	 entry;
	LIST_HEAD(, icb_session) sess;
	char			 name[ICB_MAXGRPLEN];
	char			 mpass[ICB_MAXPASSLEN];
	struct icb_session	*moder;
};

LIST_HEAD(icb_grlist, icb_group) groups;

struct icb_cmdarg {
	struct icb_session	*sess;
	char			*cmd;
	char			*arg;
	char			*mid;
};

struct icbd_callbacks {
	void 			(*drop)(struct icb_session *, char *);
	void 			(*log)(struct icb_session *, int, char *);
	void 			(*send)(struct icb_session *, char *, size_t);
};

#ifndef nitems
#define nitems(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#endif

/* cmd.c */
void *icb_cmd_lookup(char *);

/* icb.c */
struct icb_group *icb_addgroup(struct icb_session *, char *, char *);
void icb_cmdout(struct icb_session *, int, char *);
void icb_delgroup(struct icb_group *);
void icb_error(struct icb_session *, char *);
void icb_init(struct icbd_callbacks *);
void icb_input(struct icb_session *, char *, size_t);
int  icb_ismoder(struct icb_group *, struct icb_session *);
int  icb_passmoder(struct icb_group *, struct icb_session *,
         struct icb_session *);
void icb_privmsg(struct icb_session *, char *, char *);
void icb_remove(struct icb_session *, char *);
void icb_start(struct icb_session *);
void icb_status(struct icb_session *, int , char *);
void icb_status_group(struct icb_group *, struct icb_session *, int , char *);

#endif	/* __ICB_H__ */
