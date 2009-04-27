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

#ifndef lint
static const char rcsid[] = "$ABSD: icb.c,v 1.1 2009/04/27 15:41:25 uid1005 Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <sys/queue.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "icb.h"
#include "icbd.h"

extern int creategroups;

void   icb_command(struct icb_session *, char *, char *, char *);
void   icb_exit(struct icb_session *);
int    icb_fields(char *, size_t);
void   icb_groupmsg(struct icb_session *, char *);
void   icb_login(struct icb_session *, char *, char *, char *);
char  *icb_parse(char *, size_t, int *);
void   icb_pong(struct icb_session *, char *);
void   icb_who(struct icb_session *);
/* pointers to upper level functions */
void (*icb_drop)(struct icb_session *, char *);
void (*icb_log)(struct icb_session *, int, char *);
void (*icb_send)(struct icb_session *, char *, size_t);

/*
 *  icb_init: initializes pointers to callbacks
 */
void
icb_init(struct icbd_callbacks *ic)
{
	icb_drop = ic->drop;
	icb_log = ic->log;
	icb_send = ic->send;

	LIST_INIT(&groups);
}

/*
 *  icb_start: called upon accepting a new connection, greets new client
 */
void
icb_start(struct icb_session *is)
{
	char buf[ICB_MSGSIZE];
	char hname[MAXHOSTNAMELEN];
	unsigned char buflen;

	bzero(hname, sizeof(hname));
	(void)gethostname(hname, sizeof(hname));
	buflen = snprintf(buf, sizeof(buf), "%c%c%c%c%s%c%s", '?', ICB_M_PROTO,
	    '1', ICB_M_SEP, hname, ICB_M_SEP, "icbd");
	buf[0] = buflen;
	icb_send(is, buf, buflen + 1);
	SETF(is->flags, ICB_SF_PROTOSENT);
}

/*
 *  icb_input: main input processing routine
 */
void
icb_input(struct icb_session *is, char *msg, size_t msglen)
{
	char *fields[ICB_MAXFIELDS];
	char type = msg[1];
	int i, nf, pos = 0;

	is->last = getmonotime();
	if (msglen != (size_t)((unsigned char)msg[0] + 1)) {
		icb_error(is, "Invalid packet length");
		return;
	}
	if (!ISSETF(is->flags, ICB_SF_LOGGEDIN) && type != ICB_M_LOGIN) {
		icb_error(is, "Not logged in");
		return;
	}
	if ((nf = icb_fields(msg+2, msglen-2)) >= ICB_MAXFIELDS) {
		icb_error(is, "Too much fields in the packet");
		return;
	}
	bzero(fields, sizeof(fields));
	for (i = 0; i < nf; i++)
		fields[i] = icb_parse(msg+2, msglen-2, &pos);
	switch (type) {
	case ICB_M_LOGIN:
		if (nf < 4)
			goto inputerr;
		if (strcmp(fields[3], "login") == 0)
			icb_login(is, fields[2], fields[1], fields[0]);
		else if (fields[3][0] == 'w')
			icb_who(is);
		else
			goto inputerr;
		break;
	case ICB_M_OPEN:
		icb_groupmsg(is, fields[0]);
		break;
	case ICB_M_COMMAND:
 		icb_command(is, fields[0], fields[1], fields[2]);
		break;
	case ICB_M_PROTO:
		/* ignore */
		break;
	case ICB_M_PING:
		icb_pong(is, fields[0]);
		break;
	case ICB_M_NOOP:
		/* ignore */
		break;
	/* NOT VALID */
	case ICB_M_PERSONAL:
	case ICB_M_STATUS:
	case ICB_M_ERROR:
	case ICB_M_IMPORTANT:
	case ICB_M_EXIT:
	case ICB_M_CMDOUT:
	case ICB_M_BEEP:
	case ICB_M_PONG:
	default:
		icb_error(is, "Bummer. This is a bummer, man.");
		return;
	}
	return;
inputerr:
	icb_error(is, "Malformed packet");
}

/*
 *  icb_login: handles login ('a') packets
 */
void
icb_login(struct icb_session *is, char *group, char *nick, char *client)
{
	char buf[ICB_MSGSIZE];
	char host[MAXHOSTNAMELEN];
	struct icb_group *ig;
	struct icb_session *s;

	LIST_FOREACH(ig, &groups, entry) {
		if (strcmp(ig->name, group) == 0)
			break;
	}
	if (ig == NULL) {
		if (!creategroups) {
			icb_error(is, "Invalid group");
			return;
		} else {
			ig = icb_addgroup(is, group, NULL);
			(void)snprintf(buf, sizeof(buf), "%s created group %s",
			    nick, group);
			icb_log(is, ICB_LOG_NORMAL, buf);
		}
	}
	LIST_FOREACH(s, &ig->sess, entry) {
		if (strcmp(s->nick, nick) == 0) {
			icb_error(is, "Nick is already in use");
			return;
		}
	}
	/* getpeerhost is here cause some clients don't send replies to proto */
	getpeerhost(is, host, sizeof(host));
	if (strlen(host) > 0)
		strlcpy(is->host, host, sizeof(is->host));
	if (client && strlen(client) > 0)
		strlcpy(is->client, client, sizeof(is->client));
	strlcpy(is->nick, nick, sizeof(is->nick));
	is->group = ig;
	is->login = is->last = getmonotime();

	/* notify group */
	(void)snprintf(buf, sizeof(buf), "%s (%s@%s) entered group", is->nick,
	    is->client, is->host);
	icb_status_group(ig, NULL, STATUS_SIGNON, buf);

	CLRF(is->flags, ICB_SF_PROTOSENT);
	SETF(is->flags, ICB_SF_LOGGEDIN);

	LIST_INSERT_HEAD(&ig->sess, is, entry);

	/* acknowledge successful login */
	buf[0] = 1;
	buf[1] = ICB_M_LOGIN;
	icb_send(is, buf, 2);

	/* notify user */
	(void)snprintf(buf, sizeof(buf), "You're now in group %s%s", ig->name,
	    ig->moder == is ? " as moderator" : "");
	icb_status(is, STATUS_STATUS, buf);
}

/*
 *  icb_who: sends back the list of users
 */
void
icb_who(struct icb_session *is)
{
	icb_error(is, "Who is not implemented");
	icb_exit(is);
}

/*
 *  icb_groupmsg: handles open message ('b') packets
 */
void
icb_groupmsg(struct icb_session *is, char *msg)
{
	char buf[ICB_MSGSIZE];
	struct icb_group *ig = is->group;
	struct icb_session *s;
	unsigned char buflen;

	buflen = snprintf(buf, sizeof(buf), "%c%c%s%c%s", '?', ICB_M_OPEN,
	    is->nick, ICB_M_SEP, msg);
	buf[0] = buflen;

	LIST_FOREACH(s, &ig->sess, entry) {
		if (s == is)
			continue;
		icb_send(s, buf, buflen + 1);
	}
}

/*
 *  icb_privmsg: handles personal message ('c') packets
 */
void
icb_privmsg(struct icb_session *is, char *whom, char *msg)
{
	char buf[ICB_MSGSIZE];
	struct icb_group *ig = is->group;
	struct icb_session *s;
	unsigned char buflen;

	LIST_FOREACH(s, &ig->sess, entry) {
		if (strcmp(s->nick, whom) == 0)
			break;
	}
	if (s == NULL) {
		icb_error(is, "No such user");
		return;
	}
	buflen = snprintf(buf, sizeof(buf), "%c%c%s%c%s", '?', ICB_M_PERSONAL,
	    is->nick, ICB_M_SEP, msg);
	buf[0] = buflen;
	icb_send(s, buf, buflen + 1);
}

/*
 *  icb_pong: replies to the ping ('l') packets
 */
void
icb_pong(struct icb_session *is, char *mid)
{
	char buf[ICB_MSGSIZE];
	unsigned char buflen;

	buflen = snprintf(buf, sizeof(buf), "%c%c%s", '?', ICB_M_PONG, mid);
	buf[0] = buflen;
	icb_send(is, buf, buflen + 1);
}

/*
 *  icb_exit: sends a client exit ('g') packet
 */
void
icb_exit(struct icb_session *is)
{
	char buf[] = { 1, ICB_M_EXIT };

	icb_send(is, buf, sizeof(buf));
}

/*
 *  icb_command: handles command ('h') packets
 */
void
icb_command(struct icb_session *is, char *cmd, char *arg, char *mid)
{
	void (*handler)(struct icb_cmdarg *);
	struct icb_cmdarg ca = { is, cmd, arg, mid };

	if ((handler = icb_cmd_lookup(cmd)) == NULL) {
		icb_error(is, "No handler found, sorry");
		return;
	}
	handler(&ca);
}	

/*
 *  icb_cmdout: sends out command output ('i') packets, called from the
 *              command handlers
 */
void
icb_cmdout(struct icb_session *is, int type, char *outmsg)
{
	char buf[ICB_MSGSIZE];
	unsigned char buflen;
	char *otype = NULL;

	switch (type) {
	case CMDOUT_CO:
		otype = "co";
		break;
	case CMDOUT_EC:
		otype = "ec";
		break;
	case CMDOUT_WL:
		otype = "wl";
		break;
	case CMDOUT_WG:
		otype = "wg";
		break;
	default:
		icb_log(is, ICB_LOG_ERROR, "icb_cmdout: unknown cmdout type");
		return;
	}
	buflen = snprintf(buf, sizeof(buf), "%c%c%s%c%s", '?', ICB_M_CMDOUT,
	    otype, ICB_M_SEP, outmsg);
	buf[0] = buflen;
	icb_send(is, buf, buflen + 1);
}

/*
 *  icb_status: sends a status message ('d') to the client
 */
void
icb_status(struct icb_session *is, int type, char *statmsg)
{
	char buf[ICB_MSGSIZE];
	unsigned char buflen;
	struct /*icb_statmsg*/ {
		int		 type;
		const char	*msg;
	} statmsgtab[] = {
		{ STATUS_AWAY,		"Away" },
		{ STATUS_ARRIVE,	"Arrive" },
		{ STATUS_DEPART,	"Depart" },
		{ STATUS_NOTIFY,	"Notify" },
		{ STATUS_SIGNON,	"Sign-on" },
		{ STATUS_SIGNOFF,	"Sign-off" },
		{ STATUS_STATUS,	"Status" },
		{ STATUS_WARNING,	"Warning" },
		{ NULL,			NULL }
	};

	if (type < 0 || type > (int)nitems(statmsgtab) - 1)
		return;

	buflen = snprintf(buf, sizeof(buf), "%c%c%s%c%s", '?', ICB_M_STATUS,
	    statmsgtab[type].msg, ICB_M_SEP, statmsg);
	buf[0] = buflen;
	icb_log(is, ICB_LOG_DEBUG, statmsg);
	icb_send(is, buf, buflen + 1);
}

/*
 *  icb_status: sends a status message ('d') to the group except of the
 *              "ex" if it's not NULL
 */
void
icb_status_group(struct icb_group *ig, struct icb_session *ex, int type,
    char *statmsg)
{
	struct icb_session *s;

	LIST_FOREACH(s, &ig->sess, entry) {
		if (ex && s == ex)
			continue;
		icb_status(s, type, statmsg);
	}
}

/*
 *  icb_error: sends an error message ('e') to the client
 */
void
icb_error(struct icb_session *is, char *errmsg)
{
	char buf[ICB_MSGSIZE];
	unsigned char buflen;

	buflen = snprintf(buf, sizeof(buf), "%c%c%s", '?', ICB_M_ERROR,
	    errmsg);
	buf[0] = buflen;
	icb_log(is, ICB_LOG_DEBUG, errmsg);
	icb_send(is, buf, buflen + 1);
}

/*
 *  icb_remove: removes a session from the associated group
 */
void
icb_remove(struct icb_session *is, char *reason)
{
	char *msg = NULL;

	if (is->group) {
		if (is->group->moder && is->group->moder == is)
			is->group->moder = NULL;
		LIST_REMOVE(is, entry);
		(void)asprintf(&msg, "%s (%s@%s) just left: %s", is->nick,
		    is->client, is->host, reason);
		icb_status_group(is->group, NULL, STATUS_SIGNOFF, msg);
		free(msg);
	} else
		icb_log(is, ICB_LOG_ERROR,
		    "icb_remove: session isn't attached to any group");
}

/*
 *  icb_addgroup: adds a new group to the list
 */
struct icb_group *
icb_addgroup(struct icb_session *is, char *name, char *mpass)
{
	struct icb_group *ig;

	if ((ig = calloc(1, sizeof(*ig))) == NULL)
		err(1, NULL);
	strlcpy(ig->name, name, sizeof(ig->name));
	if (mpass)
		strlcpy(ig->mpass, mpass, sizeof(ig->mpass));
	if (is)
		ig->moder = is;
	LIST_INIT(&ig->sess);
	LIST_INSERT_HEAD(&groups, ig, entry);
	return (ig);
}

/*
 *  icb_delgroup: removes a group from the list
 */
void
icb_delgroup(struct icb_group *ig)
{
	struct icb_session *s;

	/* well, i guess we should kick out participants! ;-) */
	LIST_FOREACH(s, &ig->sess, entry) {
		icb_status(s, STATUS_WARNING, "Group dismissed");
		icb_exit(s);
		s->group = NULL;
	}
	LIST_REMOVE(ig, entry);
	bzero(ig, sizeof(ig));	/* paranoic thing, obviously */
	free(ig);
}

/*
 *  icb_fields: counts number of fields separated with '\001'
 */
int
icb_fields(char *msg, size_t msglen)
{
	size_t i;
	int n = 1;

	for (i = 0; i < msglen; i++)
		if (msg[i] == ICB_M_SEP)
			n++;	
	return (n);
}

/*
 *  icb_parse: scans message for separators and returns pointer to the
 *             beginning of the field at position specified by 'pos'
 */
char *
icb_parse(char *msg, size_t len, int *pos)
{
	char *s = msg + *pos;
	char *t = s;
	size_t i;

	if ((size_t)*pos > len)
		return (NULL);
	for (i = 0; i < len - *pos; i++) {
		if (t[i] == ICB_M_SEP) {
			t[i] = '\0';
			break;
		}
	}
	*pos += i + 1;
	return (s);
}
