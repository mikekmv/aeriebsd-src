/*
 * Copyright (c) 2009, 2010 Mike Belopuhov
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
static const char rcsid[] = "$ABSD: cmd.c,v 1.20 2010/01/03 01:30:00 kmerz Exp $";
#endif /* not lint */

#include <sys/param.h>
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <event.h>

#include "icb.h"

extern int creategroups;

void icb_cmd_boot(struct icb_session *, char *);
void icb_cmd_change(struct icb_session *, char *);
void icb_cmd_personal(struct icb_session *, char *);
void icb_cmd_pass(struct icb_session *, char *);
void icb_cmd_topic(struct icb_session *, char *);
void icb_cmd_who(struct icb_session *, char *);

extern void (*icb_drop)(struct icb_session *, char *);
extern void (*icb_log)(struct icb_session *, int, const char *, ...);
extern void (*icb_send)(struct icb_session *, char *, size_t);

void *
icb_cmd_lookup(char *cmd)
{
	struct {
		const char	*cmd;
		void		(*handler)(struct icb_session *, char *);
	} cmdtab[] = {
		{ "boot",	icb_cmd_boot },
		{ "g",		icb_cmd_change },
		{ "m",		icb_cmd_personal },
		{ "msg",	icb_cmd_personal },
		{ "pass",	icb_cmd_pass },
		{ "topic",	icb_cmd_topic },
		{ "w",		icb_cmd_who },
		{ NULL,		NULL }
	};
	int i;

	for (i = 0; cmdtab[i].cmd != NULL; i++)
		if (strcasecmp(cmdtab[i].cmd, cmd) == 0)
			return (cmdtab[i].handler);
	return (NULL);
}

void
icb_cmd_boot(struct icb_session *is, char *arg)
{
	char buf[ICB_MAXNICKLEN + 12];
	struct icb_group *ig;
	struct icb_session *s;

	/* to boot or not to boot, that is the question */
	ig = is->group;
	if (!icb_ismoder(ig, is)) {
		icb_status(is, STATUS_NOTIFY, "Sorry, booting is a privilege "
		    "you don't possess");
		return;
	}

	/* who would be a target then? */
	LIST_FOREACH(s, &ig->sess, entry) {
		if (strcmp(s->nick, arg) == 0)
			break;
	}
	if (s == NULL) {
		icb_status(is, STATUS_NOTIFY, "No such user");
		return;
	}

	/* okay, here we go, but first, be polite and notify a user */
	(void)snprintf(buf, sizeof buf, "%s booted you", is->nick);
	icb_status(s, STATUS_BOOT, buf);
	(void)snprintf(buf, sizeof buf, "%s was booted", s->nick);
	icb_status_group(s->group, s, STATUS_BOOT, buf);
	icb_drop(s, "booted");
}

void
icb_cmd_change(struct icb_session *is, char *arg)
{
	char buf[ICB_MSGSIZE];
	struct icb_group *ig;
	struct icb_session *s;
	int changing = 0;

	if (strlen(arg) == 0) {
		icb_error(is, "Invalid group");
		return;
	}

	LIST_FOREACH(ig, &groups, entry) {
		if (strcmp(ig->name, arg) == 0)
			break;
	}
	if (ig == NULL) {
		if (!creategroups) {
			icb_error(is, "Invalid group");
			return;
		} else {
			if ((ig = icb_addgroup(is, arg, NULL)) == NULL) {
				icb_error(is, "Can't create group");
				return;
			}
			icb_log(NULL, LOG_DEBUG, "%s created group %s",
			    is->nick, arg);
		}
	}

	/* changing to the same group is strictly prohibited */
	if (is->group && is->group == ig) {
		icb_error(is, "Huh?");
		return;
	}

	LIST_FOREACH(s, &ig->sess, entry) {
		if (strcmp(s->nick, is->nick) == 0) {
			icb_error(is, "Nick is already in use");
			return;
		}
	}

	if (is->group) {
		changing = 1;
		if (icb_ismoder(is->group, is))
			(void)icb_pass(is->group, is, NULL);
		LIST_REMOVE(is, entry);
		(void)snprintf(buf, sizeof buf, "%s (%s@%s) just left",
		    is->nick, is->client, is->host);
		icb_status_group(is->group, NULL, STATUS_DEPART, buf);
	}

	is->group = ig;
	LIST_INSERT_HEAD(&ig->sess, is, entry);

	/* notify group */
	(void)snprintf(buf, sizeof buf, "%s (%s@%s) entered group", is->nick,
	    is->client, is->host);
	icb_status_group(ig, is, changing ? STATUS_ARRIVE : STATUS_SIGNON, buf);

	/* acknowledge successful join */
	(void)snprintf(buf, sizeof buf, "You are now in group %s%s", ig->name,
	    icb_ismoder(ig, is) ? " as moderator" : "");
	icb_status(is, STATUS_STATUS, buf);

	/* send user a topic name */
	if (strlen(ig->topic) > 0) {
		(void)snprintf(buf, sizeof buf, "The topic is: %s", ig->topic);
		icb_status(is, STATUS_TOPIC, buf);
	}
}

void
icb_cmd_personal(struct icb_session *is, char *arg)
{
	char *p;

	if ((p = strchr(arg, ' ')) == 0) {
		icb_error(is, "Empty message");
		return;
	}
	*p = '\0';
	icb_privmsg(is, arg, ++p);
}

void
icb_cmd_pass(struct icb_session *is, char *arg __attribute__((unused)))
{
	if (!icb_ismoder(is->group, is))
		(void)icb_pass(is->group, is->group->moder, is);
}

void
icb_cmd_topic(struct icb_session *is, char *arg)
{
	char buf[ICB_MAXGRPLEN + ICB_MAXTOPICLEN + 25];
	struct icb_group *ig = is->group;

	if (strlen(arg) == 0) {	/* querying the topic */
		if (strlen(ig->topic) > 0)
			(void)snprintf(buf, sizeof buf, "The topic is: %s",
			    ig->topic);
		else
			(void)snprintf(buf, sizeof buf,
			    "The topic is not set.");
		icb_status(is, STATUS_TOPIC, buf);
	} else {		/* setting the topic */
		strlcpy(ig->topic, arg, sizeof ig->topic);
		(void)snprintf(buf, sizeof buf,
		    "%s changed the topic to \"%s\"", is->nick, ig->topic);
		icb_status_group(ig, NULL, STATUS_TOPIC, buf);
	}
}

void
icb_cmd_who(struct icb_session *is, char *arg __attribute__((unused)))
{
	icb_who(is, NULL);
}
