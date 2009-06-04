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
static const char rcsid[] = "$ABSD: icb.c,v 1.18 2009/06/04 12:10:28 mikeb Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <sys/queue.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "icb.h"
#include "icbd.h"

extern int creategroups;

void   icb_command(struct icb_session *, char *, char *, char *);
int    icb_fields(char *, size_t);
void   icb_groupmsg(struct icb_session *, char *);
void   icb_login(struct icb_session *, char *, char *, char *);
char  *icb_parse(char *, size_t, size_t *);
/* pointers to upper level functions */
void (*icb_drop)(struct icb_session *, char *);
void (*icb_log)(struct icb_session *, int, const char *, ...);
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
	char buf[MAXHOSTNAMELEN + 9 + 1];
	char hname[MAXHOSTNAMELEN];
	int buflen;

	bzero(hname, sizeof hname);
	(void)gethostname(hname, sizeof hname);
	buflen = snprintf(&buf[1], sizeof buf - 1, "%c%c%c%s%c%s", ICB_M_PROTO,
	    '1', ICB_M_SEP, hname, ICB_M_SEP, "icbd");
	buf[0] = ++buflen;
	icb_send(is, buf, buflen + 1);
	SETF(is->flags, ICB_SF_PROTOSENT);
}

/*
 *  icb_input: main input processing routine
 */
void
icb_input(struct icb_session *is, char *msg,
    size_t msglen __attribute__((unused)))
{
	char *fields[ICB_MAXFIELDS];
	char type;
	size_t datalen, pos = 0;
	int i, nf;

	is->last = getmonotime();
	datalen = (size_t)(unsigned char)msg[0] - 1;
	type = msg[1];
	msg += 2;
	if (!ISSETF(is->flags, ICB_SF_LOGGEDIN) && type != ICB_M_LOGIN) {
		icb_error(is, "Not logged in");
		return;
	}
	if ((nf = icb_fields(msg, datalen)) >= ICB_MAXFIELDS) {
		icb_error(is, "Too much fields in the packet");
		return;
	}
	bzero(fields, sizeof fields);
	for (i = 0; i < nf; i++)
		fields[i] = icb_parse(msg, datalen, &pos);
	switch (type) {
	case ICB_M_LOGIN:
		if (nf < 4)
			goto inputerr;
		if (fields[3][0] == 'w') {
			icb_error(is, "Command not implemented");
			icb_drop(is, NULL);
			return;
		} else if (strcmp(fields[3], "login") != 0)
			goto inputerr;
		icb_login(is, fields[2], fields[1], fields[0]);
		break;
	case ICB_M_OPEN:
		icb_groupmsg(is, fields[0]);
		break;
	case ICB_M_COMMAND:
 		icb_command(is, fields[0], fields[1], fields[2]);
		break;
	case ICB_M_PROTO:
	case ICB_M_NOOP:
		/* ignore */
		break;
	default:
		/* everything else is not valid */
		icb_error(is, "Bummer. This is a bummer, man.");
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
	char *defgrp = "1";
	struct icb_group *ig;
	struct icb_session *s;

	if (!group || strlen(group) == 0)
		group = defgrp;
	LIST_FOREACH(ig, &groups, entry) {
		if (strcmp(ig->name, group) == 0)
			break;
	}
	if (ig == NULL) {
		if (!creategroups) {
			icb_error(is, "Invalid group %s", group);
			icb_drop(is, NULL);
			return;
		} else {
			if ((ig = icb_addgroup(is, group, NULL)) == NULL) {
				icb_error(is, "Can't create group %s", group);
				return;
			}
			icb_log(NULL, ICB_LOG_DEBUG, "%s created group %s",
			    nick, group);
		}
	}
	LIST_FOREACH(s, &ig->sess, entry) {
		if (strcmp(s->nick, nick) == 0) {
			icb_error(is, "Nick is already in use");
			icb_drop(is, NULL);
			return;
		}
	}

	if (client && strlen(client) > 0)
		strlcpy(is->client, client, sizeof is->client);
	strlcpy(is->nick, nick, sizeof is->nick);
	is->group = ig;
	is->login = time(NULL);
	is->last = getmonotime();

	/* notify group */
	(void)snprintf(buf, sizeof buf, "%s (%s@%s) entered group", is->nick,
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
	(void)snprintf(buf, sizeof buf, "You're now in group %s%s", ig->name,
	    icb_ismoder(ig, is) ? " as moderator" : "");
	icb_status(is, STATUS_STATUS, buf);

	/* send user a topic name */
	if (strlen(ig->topic) > 0) {
		(void)snprintf(buf, sizeof buf, "Topic for %s is \"%s\"",
		    ig->name, ig->topic);
		icb_status(is, STATUS_TOPIC, buf);
	}
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
	int buflen;

	buflen = snprintf(&buf[1], sizeof buf - 1, "%c%s%c%s", ICB_M_OPEN,
	    is->nick, ICB_M_SEP, msg);
	buf[0] = ++buflen;

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
	int buflen;

	LIST_FOREACH(s, &ig->sess, entry) {
		if (strcmp(s->nick, whom) == 0)
			break;
	}
	if (!s) {
		icb_error(is, "No such user");
		return;
	}
	buflen = snprintf(&buf[1], sizeof buf - 1, "%c%s%c%s", ICB_M_PERSONAL,
	    is->nick, ICB_M_SEP, msg);
	buf[0] = ++buflen;
	icb_send(s, buf, buflen + 1);
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
		icb_error(is, "Unsupported command");
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
	int buflen;
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
		icb_log(is, ICB_LOG_ERROR, "unknown cmdout type");
		return;
	}
	buflen = snprintf(&buf[1], sizeof buf - 1, "%c%s%c%s", ICB_M_CMDOUT,
	    otype, ICB_M_SEP, outmsg);
	buf[0] = ++buflen;
	icb_send(is, buf, buflen + 1);
}

/*
 *  icb_status: sends a status message ('d') to the client
 */
void
icb_status(struct icb_session *is, int type, char *statmsg)
{
	char buf[ICB_MSGSIZE];
	int buflen;
	static struct {
		int		 type;
		const char	*msg;
	} msgtab[] = {
		{ STATUS_AWAY,		"Away" },
		{ STATUS_ARRIVE,	"Arrive" },
		{ STATUS_BOOT,		"Boot" },
		{ STATUS_DEPART,	"Depart" },
		{ STATUS_NOAWAY,	"NoAway" },
		{ STATUS_NOTIFY,	"Notify" },
		{ STATUS_SIGNON,	"Sign-on" },
		{ STATUS_SIGNOFF,	"Sign-off" },
		{ STATUS_STATUS,	"Status" },
		{ STATUS_TOPIC,		"Topic" },
		{ STATUS_WARNING,	"Warning" },
		{ NULL,			NULL }
	};

	if (type < 0 || type > (int)nitems(msgtab) - 1)
		return;

	buflen = snprintf(&buf[1], sizeof buf - 1, "%c%s%c%s", ICB_M_STATUS,
	    msgtab[type].msg, ICB_M_SEP, statmsg);
	buf[0] = ++buflen;
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
	icb_log(NULL, ICB_LOG_DEBUG, "%s", statmsg);
}

/*
 *  icb_error: sends an error message ('e') to the client
 */
void
icb_error(struct icb_session *is, const char *fmt, ...)
{
	char buf[ICB_MSGSIZE];
	va_list ap;
	int buflen = 1;

	va_start(ap, fmt);
	buflen += vsnprintf(&buf[2], sizeof buf - 2, fmt, ap);
	va_end(ap);
	buf[0] = ++buflen;
	buf[1] = ICB_M_ERROR;
	icb_send(is, buf, buflen + 1);
	icb_log(is, ICB_LOG_DEBUG, "%s", buf + 2);
}

/*
 *  icb_remove: removes a session from the associated group
 */
void
icb_remove(struct icb_session *is, char *reason)
{
	char buf[ICB_MSGSIZE];

	if (is->group) {
		if (icb_ismoder(is->group, is))
			(void)icb_pass(is->group, is, NULL);
		LIST_REMOVE(is, entry);
		if (reason)
			(void)snprintf(buf, sizeof buf, "%s (%s@%s) just left"
			    ": %s", is->nick, is->client, is->host, reason);
		else
			(void)snprintf(buf, sizeof buf, "%s (%s@%s) just left",
			    is->nick, is->client, is->host);
		icb_status_group(is->group, NULL, STATUS_SIGNOFF, buf);
	}
}

/*
 *  icb_addgroup: adds a new group to the list
 */
struct icb_group *
icb_addgroup(struct icb_session *is, char *name, char *mpass)
{
	struct icb_group *ig;

	if ((ig = calloc(1, sizeof *ig)) == NULL)
		return (NULL);
	strlcpy(ig->name, name, sizeof ig->name);
	if (mpass)
		strlcpy(ig->mpass, mpass, sizeof ig->mpass);
	if (is)
		ig->moder = is;
	LIST_INIT(&ig->sess);
	LIST_INSERT_HEAD(&groups, ig, entry);
	return (ig);
}

#ifdef notused
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
		s->group = NULL;
	}
	LIST_REMOVE(ig, entry);
	bzero(ig, sizeof ig);	/* paranoic thing, obviously */
	free(ig);
}
#endif

/*
 *  icb_who: sends a list of users of the specified group (or the current
 *           one otherwise) in the "wl" format
 */
void
icb_who(struct icb_session *is, struct icb_group *ig)
{
	char buf[ICB_MSGSIZE];
	struct icb_session *s;

	if (!ig)
		ig = is->group;
	LIST_FOREACH(s, &ig->sess, entry) {
		(void)snprintf(buf, sizeof buf,
		    "%c%c%s%c%d%c0%c%d%c%s%c%s%c%s",
		    icb_ismoder(ig, s) ? '*' : ' ', ICB_M_SEP,
		    s->nick, ICB_M_SEP, getmonotime() - s->last,
		    ICB_M_SEP, ICB_M_SEP, s->login, ICB_M_SEP,
		    s->client, ICB_M_SEP, s->host, ICB_M_SEP,
		    ISSETF(s->flags, ICB_SF_AWAY) ? "[away]" : "");
		icb_cmdout(is, CMDOUT_WL, buf);
	}
}

/*
 *  icb_ismoder: checks whether group is moderated by "is"
 */
int
icb_ismoder(struct icb_group *ig, struct icb_session *is)
{
	if (ig->moder && ig->moder == is)
		return (1);
	return (0);
}

/*
 *  icb_pass: passes moderation of group "ig" from "from" to "to",
 *            returns -1 if "from" is not a moderator, 1 if passed
 *            to "to" and 0 otherwise (no moderator or passed to the
 *            internal bot)
 */
int
icb_pass(struct icb_group *ig, struct icb_session *from,
    struct icb_session *to)
{
	char buf[ICB_MSGSIZE];

	if (ig->moder && ig->moder != from)
		return (-1);
	if (!from && !to)
		return (-1);
	ig->moder = to;
	if (to) {
		(void)snprintf(buf, sizeof buf, "%s just passed you moderation"
		    " of %s", from ? from->nick : "server", ig->name);
		icb_status(to, STATUS_NOTIFY, buf);
	}
	(void)snprintf(buf, sizeof buf, "%s has passed moderation "
	    "to %s", from ? from->nick : "server", to ? to->nick : "server");
	icb_status_group(ig, to, STATUS_NOTIFY, buf);
	return (1);
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
icb_parse(char *msg, size_t len, size_t *pos)
{
	char *s = msg + *pos;
	char *t = s;
	size_t i;

	if (*pos > len)
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
