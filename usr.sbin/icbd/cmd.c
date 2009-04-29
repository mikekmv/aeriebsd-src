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
static const char rcsid[] = "$ABSD: cmd.c,v 1.2 2009/04/29 15:18:46 mikeb Exp $";
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

extern int creategroups;

void icb_cmd_boot(struct icb_cmdarg *);
void icb_cmd_group(struct icb_cmdarg *);
void icb_cmd_help(struct icb_cmdarg *);
void icb_cmd_personal(struct icb_cmdarg *);
void icb_cmd_motd(struct icb_cmdarg *);
void icb_cmd_pass(struct icb_cmdarg *);
void icb_cmd_status(struct icb_cmdarg *);
void icb_cmd_topic(struct icb_cmdarg *);
void icb_cmd_wall(struct icb_cmdarg *);
void icb_cmd_who(struct icb_cmdarg *);

extern void (*icb_drop)(struct icb_session *, char *);
extern void (*icb_log)(struct icb_session *, int, char *);
extern void (*icb_write)(struct icb_session *, char *, size_t);

struct icbcmd {
	const char	*cmd;
	void		(*handler)(struct icb_cmdarg *);
} cmdtab[] = {
	{ "boot",	icb_cmd_boot },
	{ "g",		icb_cmd_group },
	{ "help",	icb_cmd_help },
	{ "m",		icb_cmd_personal },
	{ "msg",	icb_cmd_personal },
	{ "motd",	icb_cmd_motd },
	{ "pass",	icb_cmd_pass },
	{ "status",	icb_cmd_status },
	{ "topic",	icb_cmd_topic },
	{ "wall",	icb_cmd_wall },
	{ "w",		icb_cmd_who },
	{ NULL,		NULL }
};

void *
icb_cmd_lookup(char *cmd)
{
	struct icbcmd *ic = cmdtab;
	int i;

	for (i = 0; ic->cmd != NULL; ic = &cmdtab[++i])
		if (strcasecmp(ic->cmd, cmd) == 0)
			return (ic->handler);
	return (NULL);
}

void
icb_cmd_boot(struct icb_cmdarg *ca)
{
}

void
icb_cmd_group(struct icb_cmdarg *ca)
{
	char buf[ICB_MSGSIZE];
	struct icb_group *ig;
	struct icb_session *is, *s;

	is = ca->sess;
	LIST_FOREACH(ig, &groups, entry) {
		if (strcmp(ig->name, ca->arg) == 0)
			break;
	}
	if (ig == NULL) {
		if (!creategroups) {
			icb_error(is, "Invalid group");
			return;
		} else {
			ig = icb_addgroup(is, ca->arg, NULL);
			(void)snprintf(buf, sizeof(buf), "%s created group %s",
			    is->nick, ca->arg);
			icb_log(is, ICB_LOG_NORMAL, buf);
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
		(void)snprintf(buf, sizeof(buf), "%s (%s@%s) just left",
		    is->nick, is->client, is->host);
		icb_status_group(is->group, is, STATUS_DEPART, buf);
		LIST_REMOVE(is, entry);
	}

	is->group = ig;
	LIST_INSERT_HEAD(&ig->sess, is, entry);

	/* notify group */
	(void)snprintf(buf, sizeof(buf), "%s (%s@%s) entered group", is->nick,
	    is->client, is->host);
	icb_status_group(ig, is, STATUS_SIGNON, buf);

	/* acknowledge successful join */
	(void)snprintf(buf, sizeof(buf), "You're now in group %s%s", ig->name,
	    ig->moder == is ? " as moderator" : "");
	icb_status(is, STATUS_STATUS, buf);
}

void
icb_cmd_help(struct icb_cmdarg *ca)
{
}

void
icb_cmd_personal(struct icb_cmdarg *ca)
{
	char *p;

	if ((p = strchr(ca->arg, ' ')) == 0) {
		icb_error(ca->sess, "Empty message");
		return;
	}
	*p = '\0';
	icb_privmsg(ca->sess, ca->arg, ++p);
}

void
icb_cmd_motd(struct icb_cmdarg *ca)
{
}

void
icb_cmd_pass(struct icb_cmdarg *ca)
{
}

void
icb_cmd_status(struct icb_cmdarg *ca)
{
}

void
icb_cmd_topic(struct icb_cmdarg *ca)
{
}

void
icb_cmd_wall(struct icb_cmdarg *ca)
{
}

void
icb_cmd_who(struct icb_cmdarg *ca)
{
}
