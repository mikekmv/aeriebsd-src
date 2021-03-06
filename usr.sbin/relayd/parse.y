
/*
 * Copyright (c) 2007, 2008 Reyk Floeter <reyk@openbsd.org>
 * Copyright (c) 2006 Pierre-Yves Ritschard <pyr@openbsd.org>
 * Copyright (c) 2004, 2005 Esben Norby <norby@openbsd.org>
 * Copyright (c) 2004 Ryan McBride <mcbride@openbsd.org>
 * Copyright (c) 2002, 2003, 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
 * Copyright (c) 2001 Daniel Hartmeier.  All rights reserved.
 * Copyright (c) 2001 Theo de Raadt.  All rights reserved.
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

%{
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/queue.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <ctype.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include <openssl/ssl.h>

#include "relayd.h"

TAILQ_HEAD(files, file)		 files = TAILQ_HEAD_INITIALIZER(files);
static struct file {
	TAILQ_ENTRY(file)	 entry;
	FILE			*stream;
	char			*name;
	int			 lineno;
	int			 errors;
} *file, *topfile;
struct file	*pushfile(const char *, int);
int		 popfile(void);
int		 check_file_secrecy(int, const char *);
int		 yyparse(void);
int		 yylex(void);
int		 yyerror(const char *, ...);
int		 kw_cmp(const void *, const void *);
int		 lookup(char *);
int		 lgetc(int);
int		 lungetc(int);
int		 findeol(void);

TAILQ_HEAD(symhead, sym)	 symhead = TAILQ_HEAD_INITIALIZER(symhead);
struct sym {
	TAILQ_ENTRY(sym)	 entry;
	int			 used;
	int			 persist;
	char			*nam;
	char			*val;
};
int		 symset(const char *, const char *, int);
char		*symget(const char *);

struct relayd		*conf = NULL;
static int		 errors = 0;
objid_t			 last_rdr_id = 0;
objid_t			 last_table_id = 0;
objid_t			 last_host_id = 0;
objid_t			 last_relay_id = 0;
objid_t			 last_proto_id = 0;

static struct rdr	*rdr = NULL;
static struct table	*table = NULL;
static struct relay	*rlay = NULL;
static struct protocol	*proto = NULL;
static struct protonode	 node;
static u_int16_t	 label = 0;
static in_port_t	 tableport = 0;

struct address	*host_v4(const char *);
struct address	*host_v6(const char *);
int		 host_dns(const char *, struct addresslist *,
		    int, in_port_t, const char *);
int		 host(const char *, struct addresslist *,
		    int, in_port_t, const char *);

struct table	*table_inherit(struct table *);

typedef struct {
	union {
		int64_t		 number;
		char		*string;
		struct host	*host;
		struct timeval	 tv;
		struct table	*table;
		struct {
			enum digest_type	 type;
			char			*digest;
		}		 digest;
	} v;
	int lineno;
} YYSTYPE;

%}

%token	ALL APPEND BACKLOG BACKUP BUFFER CACHE CHANGE CHECK CIPHERS
%token	CODE COOKIE DEMOTE DIGEST DISABLE EXPECT EXTERNAL FILTER FORWARD
%token	FROM HASH HEADER HOST ICMP INCLUDE INTERFACE INTERVAL IP LABEL
%token	LISTEN LOADBALANCE LOG LOOKUP MARK MARKED MODE NAT NO NODELAY NOTHING
%token	ON PATH PORT PREFORK PROTO QUERYSTR REAL REDIRECT RELAY REMOVE TRAP
%token	REQUEST RESPONSE RETRY RETURN ROUNDROBIN SACK SCRIPT SEND SESSION
%token	SOCKET SSL STICKYADDR STYLE TABLE TAG TCP TIMEOUT TO UPDATES URL
%token	VIRTUAL WITH ERROR ROUTE TRANSPARENT PARENT INET INET6
%token	<v.string>	STRING
%token  <v.number>	NUMBER
%type	<v.string>	interface hostname table
%type	<v.number>	port http_type loglevel sslcache optssl mark parent
%type	<v.number>	proto_type dstmode retry log flag direction forwardmode
%type	<v.host>	host
%type	<v.tv>		timeout
%type	<v.digest>	digest
%type	<v.table>	tablespec

%%

grammar		: /* empty */
		| grammar include '\n'
		| grammar '\n'
		| grammar varset '\n'
		| grammar main '\n'
		| grammar rdr '\n'
		| grammar tabledef '\n'
		| grammar relay '\n'
		| grammar proto '\n'
		| grammar error '\n'		{ file->errors++; }
		;

include		: INCLUDE STRING		{
			struct file	*nfile;

			if ((nfile = pushfile($2, 0)) == NULL) {
				yyerror("failed to include file %s", $2);
				free($2);
				YYERROR;
			}
			free($2);

			file = nfile;
			lungetc('\n');
		}

optssl		: /*empty*/	{ $$ = 0; }
		| SSL		{ $$ = 1; }
		;

http_type	: STRING	{
			if (strcmp("https", $1) == 0) {
				$$ = 1;
			} else if (strcmp("http", $1) == 0) {
				$$ = 0;
			} else {
				yyerror("invalid check type: %s", $1);
				free($1);
				YYERROR;
			}
			free($1);
		}
		;

hostname	: /* empty */		{
			$$ = strdup("");
			if ($$ == NULL)
				fatal("calloc");
		}
		| HOST STRING	{
			if (asprintf(&$$, "Host: %s\r\n", $2) == -1)
				fatal("asprintf");
		}
		;

proto_type	: /* empty */			{ $$ = RELAY_PROTO_TCP; }
		| TCP				{ $$ = RELAY_PROTO_TCP; }
		| STRING			{
			if (strcmp("http", $1) == 0) {
				$$ = RELAY_PROTO_HTTP;
			} else if (strcmp("dns", $1) == 0) {
				$$ = RELAY_PROTO_DNS;
			} else {
				yyerror("invalid protocol type: %s", $1);
				free($1);
				YYERROR;
			}
			free($1);
		}
		;

eflags_l	: eflags comma eflags_l
		| eflags
		;

opteflags	: /* nothing */
		| eflags
		;

eflags		: STYLE STRING
		{
			if ((proto->style = strdup($2)) == NULL)
				fatal("out of memory");
			free($2);
		}
		;

port		: PORT STRING {
			struct servent	*servent;

			servent = getservbyname($2, "tcp");
			if (servent == NULL) {
				yyerror("port %s is invalid", $2);
				free($2);
				YYERROR;
			}
			$$ = servent->s_port;
			free($2);
		}
		| PORT NUMBER {
			if ($2 <= 0 || $2 >= (int)USHRT_MAX) {
				yyerror("invalid port: %d", $2);
				YYERROR;
			}
			$$ = htons($2);
		}
		;

varset		: STRING '=' STRING	{
			if (symset($1, $3, 0) == -1)
				fatal("cannot store variable");
			free($1);
			free($3);
		}
		;

sendbuf		: NOTHING		{
			table->sendbuf = NULL;
			table->sendbuf_len = 0;
		}
		| STRING		{
			table->sendbuf = strdup($1);
			if (table->sendbuf == NULL)
				fatal("out of memory");
			table->sendbuf_len = strlen(table->sendbuf);
			free($1);
		}
		;

main		: INTERVAL NUMBER	{
			if ((conf->sc_interval.tv_sec = $2) < 0) {
				yyerror("invalid interval: %d", $2);
				YYERROR;
			}
		}
		| LOG loglevel		{ conf->sc_opts |= $2; }
		| TIMEOUT timeout	{
			bcopy(&$2, &conf->sc_timeout, sizeof(struct timeval));
		}
		| PREFORK NUMBER	{
			if ($2 <= 0 || $2 > RELAY_MAXPROC) {
				yyerror("invalid number of preforked "
				    "relays: %d", $2);
				YYERROR;
			}
			conf->sc_prefork_relay = $2;
		}
		| DEMOTE STRING		{
			conf->sc_flags |= F_DEMOTE;
			if (strlcpy(conf->sc_demote_group, $2,
			    sizeof(conf->sc_demote_group))
			    >= sizeof(conf->sc_demote_group)) {
				yyerror("yyparse: demote group name too long");
				free($2);
				YYERROR;
			}
			free($2);
			if (carp_demote_init(conf->sc_demote_group, 1) == -1) {
				yyerror("yyparse: error initializing group %s",
				    conf->sc_demote_group);
				YYERROR;
			}
		}
		| SEND TRAP		{ conf->sc_flags |= F_TRAP; }
		;

loglevel	: UPDATES		{ $$ = RELAYD_OPT_LOGUPDATE; }
		| ALL			{ $$ = RELAYD_OPT_LOGALL; }
		;

rdr		: REDIRECT STRING	{
			struct rdr *srv;

			conf->sc_flags |= F_NEEDPF;
			TAILQ_FOREACH(srv, conf->sc_rdrs, entry)
				if (!strcmp(srv->conf.name, $2))
					break;
			if (srv != NULL) {
				yyerror("redirection %s defined twice", $2);
				free($2);
				YYERROR;
			}
			if ((srv = calloc(1, sizeof (*srv))) == NULL)
				fatal("out of memory");

			if (strlcpy(srv->conf.name, $2,
			    sizeof(srv->conf.name)) >=
			    sizeof(srv->conf.name)) {
				yyerror("redirection name truncated");
				free(srv);
				YYERROR;
			}
			free($2);
			srv->conf.id = ++last_rdr_id;
			srv->conf.timeout.tv_sec = RELAY_TIMEOUT;
			if (last_rdr_id == INT_MAX) {
				yyerror("too many redirections defined");
				free(srv);
				YYERROR;
			}
			rdr = srv;
		} '{' optnl rdropts_l '}'	{
			if (rdr->table == NULL) {
				yyerror("redirection %s has no table",
				    rdr->conf.name);
				YYERROR;
			}
			if (TAILQ_EMPTY(&rdr->virts)) {
				yyerror("redirection %s has no virtual ip",
				    rdr->conf.name);
				YYERROR;
			}
			conf->sc_rdrcount++;
			if (rdr->backup == NULL) {
				rdr->conf.backup_id =
				    conf->sc_empty_table.conf.id;
				rdr->backup = &conf->sc_empty_table;
			} else if (rdr->backup->conf.port !=
			    rdr->table->conf.port) {
				yyerror("redirection %s uses two different "
				    "ports for its table and backup table",
				    rdr->conf.name);
				YYERROR;
			}
			if (!(rdr->conf.flags & F_DISABLE))
				rdr->conf.flags |= F_ADD;
			TAILQ_INSERT_TAIL(conf->sc_rdrs, rdr, entry);
			tableport = 0;
			rdr = NULL;
		}
		;

rdropts_l	: rdropts_l rdroptsl nl
		| rdroptsl optnl
		;

rdroptsl	: forwardmode TO tablespec interface	{
			switch ($1) {
			case FWD_NORMAL:
				if ($4 == NULL)
					break;
				yyerror("superfluous interface");
				YYERROR;
			case FWD_ROUTE:
				if ($4 != NULL)
					break;
				yyerror("missing interface to route to");
				YYERROR;
			case FWD_TRANS:
				yyerror("no transparent forward here");
				YYERROR;
			}
			if ($4 != NULL) {
				strlcpy($3->conf.ifname, $4,
				    sizeof($3->conf.ifname));
				free($4);
			}

			if ($3->conf.check == CHECK_NOCHECK) {
				yyerror("table %s has no check", $3->conf.name);
				purge_table(conf->sc_tables, $3);
				YYERROR;
			}
			if (rdr->backup) {
				yyerror("only one backup table is allowed");
				purge_table(conf->sc_tables, $3);
				YYERROR;
			}
			if (rdr->table) {
				rdr->backup = $3;
				rdr->conf.backup_id = $3->conf.id;
			} else {
				rdr->table = $3;
				rdr->conf.table_id = $3->conf.id;
			}
			$3->conf.fwdmode = $1;
			$3->conf.rdrid = rdr->conf.id;
			$3->conf.flags |= F_USED | $1;
		}
		| LISTEN ON STRING port interface {
			if (host($3, &rdr->virts,
				 SRV_MAX_VIRTS, $4, $5) <= 0) {
				yyerror("invalid virtual ip: %s", $3);
				free($3);
				free($5);
				YYERROR;
			}
			free($3);
			free($5);
			if (rdr->conf.port == 0)
				rdr->conf.port = $4;
			tableport = rdr->conf.port;
		}
		| DISABLE		{ rdr->conf.flags |= F_DISABLE; }
		| STICKYADDR		{ rdr->conf.flags |= F_STICKY; }
		| TAG STRING {
			conf->sc_flags |= F_NEEDPF;
			if (strlcpy(rdr->conf.tag, $2,
			    sizeof(rdr->conf.tag)) >=
			    sizeof(rdr->conf.tag)) {
				yyerror("redirection tag name truncated");
				free($2);
				YYERROR;
			}
			free($2);
		}
		| SESSION TIMEOUT NUMBER		{
			if ((rdr->conf.timeout.tv_sec = $3) < 0) {
				yyerror("invalid timeout: %d", $3);
				YYERROR;
			}
		}
		| include
		;

forwardmode	: FORWARD		{ $$ = FWD_NORMAL; }
		| ROUTE			{ $$ = FWD_ROUTE; }
		| TRANSPARENT FORWARD	{ $$ = FWD_TRANS; }
		;

table		: '<' STRING '>'	{
			conf->sc_flags |= F_NEEDPF;
			if (strlen($2) >= TABLE_NAME_SIZE) {
				yyerror("invalid table name");
				free($2);
				YYERROR;
			}
			$$ = $2;
		}
		;

tabledef	: TABLE table		{
			struct table *tb;

			TAILQ_FOREACH(tb, conf->sc_tables, entry)
				if (!strcmp(tb->conf.name, $2))
					break;
			if (tb != NULL) {
				yyerror("table %s defined twice", $2);
				free($2);
				YYERROR;
			}

			if ((tb = calloc(1, sizeof (*tb))) == NULL)
				fatal("out of memory");

			(void)strlcpy(tb->conf.name, $2, sizeof(tb->conf.name));
			free($2);

			tb->conf.id = 0; /* will be set later */
			bcopy(&conf->sc_timeout, &tb->conf.timeout,
			    sizeof(struct timeval));
			TAILQ_INIT(&tb->hosts);
			table = tb;
		} tabledefopts_l 	{
			if (TAILQ_EMPTY(&table->hosts)) {
				yyerror("table %s has no hosts",
				    table->conf.name);
				YYERROR;
			}
			conf->sc_tablecount++;
			TAILQ_INSERT_TAIL(conf->sc_tables, table, entry);
		}
		;

tabledefopts_l	: tabledefopts_l tabledefopts
		| tabledefopts
		;

tabledefopts	: DISABLE		{ table->conf.flags |= F_DISABLE; }
		| '{' optnl tablelist_l '}'
		;

tablelist_l	: tablelist comma tablelist_l
		| tablelist optnl
		;

tablelist	: host			{
			$1->conf.tableid = table->conf.id;
			$1->tablename = table->conf.name;
			TAILQ_INSERT_TAIL(&table->hosts, $1, entry);
		}
		| include
		;

tablespec	: table 		{
			struct table	*tb;
			if ((tb = calloc(1, sizeof (*tb))) == NULL)
				fatal("out of memory");
			(void)strlcpy(tb->conf.name, $1, sizeof(tb->conf.name));
			free($1);
			table = tb;
		} tableopts_l		{
			struct table	*tb;
			if (table->conf.port == 0)
				table->conf.port = tableport;
			if ((tb = table_inherit(table)) == NULL)
				YYERROR;
			$$ = tb;
		}
		;

tableopts_l	: tableopts tableopts_l
		| tableopts
		;

tableopts	: CHECK tablecheck
		| port			{ table->conf.port = $1; }
		| TIMEOUT timeout	{
			bcopy(&$2, &table->conf.timeout,
			    sizeof(struct timeval));
		}
		| DEMOTE STRING		{
			table->conf.flags |= F_DEMOTE;
			if (strlcpy(table->conf.demote_group, $2,
			    sizeof(table->conf.demote_group))
			    >= sizeof(table->conf.demote_group)) {
				yyerror("yyparse: demote group name too long");
				free($2);
				YYERROR;
			}
			free($2);
			if (carp_demote_init(table->conf.demote_group, 1)
			    == -1) {
				yyerror("yyparse: error initializing group "
				    "'%s'", table->conf.demote_group);
				YYERROR;
			}
		}
		| INTERVAL NUMBER	{
			if ($2 < conf->sc_interval.tv_sec ||
			    $2 % conf->sc_interval.tv_sec) {
				yyerror("table interval must be "
				    "divisible by global interval");
				YYERROR;
			}
			table->conf.skip_cnt = ($2 / conf->sc_interval.tv_sec) - 1;
		}
		| MODE dstmode		{
			switch ($2) {
			case RELAY_DSTMODE_LOADBALANCE:
			case RELAY_DSTMODE_HASH:
				if (rdr != NULL) {
					yyerror("mode not supported "
					    "for redirections");
					YYERROR;
				}
				/* FALLTHROUGH */
			case RELAY_DSTMODE_ROUNDROBIN:
				if (rlay != NULL)
					rlay->rl_conf.dstmode = $2;
				break;
			}
		}
		;

tablecheck	: ICMP			{ table->conf.check = CHECK_ICMP; }
		| TCP			{ table->conf.check = CHECK_TCP; }
		| SSL			{
			table->conf.check = CHECK_TCP;
			conf->sc_flags |= F_SSL;
			table->conf.flags |= F_SSL;
		}
		| http_type STRING hostname CODE NUMBER {
			if ($1) {
				conf->sc_flags |= F_SSL;
				table->conf.flags |= F_SSL;
			}
			table->conf.check = CHECK_HTTP_CODE;
			if ((table->conf.retcode = $5) <= 0) {
				yyerror("invalid HTTP code: %d", $5);
				free($2);
				free($3);
				YYERROR;
			}
			if (asprintf(&table->sendbuf,
			    "HEAD %s HTTP/1.0\r\n%s\r\n", $2, $3) == -1)
				fatal("asprintf");
			free($2);
			free($3);
			if (table->sendbuf == NULL)
				fatal("out of memory");
			table->sendbuf_len = strlen(table->sendbuf);
		}
		| http_type STRING hostname digest {
			if ($1) {
				conf->sc_flags |= F_SSL;
				table->conf.flags |= F_SSL;
			}
			table->conf.check = CHECK_HTTP_DIGEST;
			if (asprintf(&table->sendbuf,
			    "GET %s HTTP/1.0\r\n%s\r\n", $2, $3) == -1)
				fatal("asprintf");
			free($2);
			free($3);
			if (table->sendbuf == NULL)
				fatal("out of memory");
			table->sendbuf_len = strlen(table->sendbuf);
			(void)strlcpy(table->conf.digest, $4.digest,
			    sizeof(table->conf.digest));
			table->conf.digest_type = $4.type;
			free($4.digest);
		}
		| SEND sendbuf EXPECT STRING optssl {
			table->conf.check = CHECK_SEND_EXPECT;
			if ($5) {
				conf->sc_flags |= F_SSL;
				table->conf.flags |= F_SSL;
			}
			if (strlcpy(table->conf.exbuf, $4,
			    sizeof(table->conf.exbuf))
			    >= sizeof(table->conf.exbuf)) {
				yyerror("yyparse: expect buffer truncated");
				free($4);
				YYERROR;
			}
			translate_string(table->conf.exbuf);
			free($4);
		}
		| SCRIPT STRING {
			table->conf.check = CHECK_SCRIPT;
			if (strlcpy(table->conf.path, $2,
			    sizeof(table->conf.path)) >=
			    sizeof(table->conf.path)) {
				yyerror("script path truncated");
				free($2);
				YYERROR;
			}
			free($2);
		}
		;

digest		: DIGEST STRING
		{
			switch (strlen($2)) {
			case 40:
				$$.type = DIGEST_SHA1;
				break;
			case 32:
				$$.type = DIGEST_MD5;
				break;
			default:
				yyerror("invalid http digest");
				free($2);
				YYERROR;
			}
			$$.digest = $2;
		}
		;

proto		: proto_type PROTO STRING	{
			struct protocol *p;

			if (strcmp($3, "default") == 0) {
				p = &conf->sc_proto_default;
			} else {
				TAILQ_FOREACH(p, conf->sc_protos, entry)
					if (!strcmp(p->name, $3))
						break;
			}
			if (p != NULL) {
				yyerror("protocol %s defined twice", $3);
				free($3);
				YYERROR;
			}
			if ((p = calloc(1, sizeof (*p))) == NULL)
				fatal("out of memory");

			if (strlcpy(p->name, $3, sizeof(p->name)) >=
			    sizeof(p->name)) {
				yyerror("protocol name truncated");
				free(p);
				YYERROR;
			}
			free($3);
			p->id = ++last_proto_id;
			p->type = $1;
			p->cache = RELAY_CACHESIZE;
			p->tcpflags = TCPFLAG_DEFAULT;
			p->sslflags = SSLFLAG_DEFAULT;
			p->tcpbacklog = RELAY_BACKLOG;
			(void)strlcpy(p->sslciphers, SSLCIPHERS_DEFAULT,
			    sizeof(p->sslciphers));
			if (last_proto_id == INT_MAX) {
				yyerror("too many protocols defined");
				free(p);
				YYERROR;
			}
			RB_INIT(&p->request_tree);
			RB_INIT(&p->response_tree);
			proto = p;
		} protopts_n			{
			conf->sc_protocount++;

			if ((proto->sslflags & SSLFLAG_VERSION) == 0) {
				yyerror("invalid SSL protocol");
				YYERROR;
			}

			TAILQ_INSERT_TAIL(conf->sc_protos, proto, entry);
		}
		;

protopts_n	: /* empty */
		| '{' '}'
		| '{' optnl protopts_l '}'
		;

protopts_l	: protopts_l protoptsl nl
		| protoptsl optnl
		;

protoptsl	: SSL sslflags
		| SSL '{' sslflags_l '}'
		| TCP tcpflags
		| TCP '{' tcpflags_l '}'
		| RETURN ERROR opteflags	{ proto->flags |= F_RETURN; }
		| RETURN ERROR '{' eflags_l '}'	{ proto->flags |= F_RETURN; }
		| LABEL STRING			{
			label = pn_name2id($2);
			free($2);
			if (label == 0) {
				yyerror("invalid protocol action label");
				YYERROR;
			}
		}
		| NO LABEL			{
			label = 0;
		}
		| direction protonode log	{
			if ($3)
				node.flags |= PNFLAG_LOG;
			node.label = label;
			if (protonode_add($1, proto, &node) == -1) {
				yyerror("failed to add protocol node");
				YYERROR;
			}
			bzero(&node, sizeof(node));
		}
		| include
		;

direction	: /* empty */		{ $$ = RELAY_DIR_REQUEST; }
		| REQUEST		{ $$ = RELAY_DIR_REQUEST; }
		| RESPONSE		{ $$ = RELAY_DIR_RESPONSE; }
		;

tcpflags_l	: tcpflags comma tcpflags_l
		| tcpflags
		;

tcpflags	: SACK			{ proto->tcpflags |= TCPFLAG_SACK; }
		| NO SACK		{ proto->tcpflags |= TCPFLAG_NSACK; }
		| NODELAY		{ proto->tcpflags |= TCPFLAG_NODELAY; }
		| NO NODELAY		{ proto->tcpflags |= TCPFLAG_NNODELAY; }
		| BACKLOG NUMBER	{
			if ($2 < 0 || $2 > RELAY_MAX_SESSIONS) {
				yyerror("invalid backlog: %d", $2);
				YYERROR;
			}
			proto->tcpbacklog = $2;
		}
		| SOCKET BUFFER NUMBER	{
			proto->tcpflags |= TCPFLAG_BUFSIZ;
			if ((proto->tcpbufsiz = $3) < 0) {
				yyerror("invalid socket buffer size: %d", $3);
				YYERROR;
			}
		}
		| IP STRING NUMBER	{
			if ($3 < 0) {
				yyerror("invalid ttl: %d", $3);
				free($2);
				YYERROR;
			}
			if (strcasecmp("ttl", $2) == 0) {
				proto->tcpflags |= TCPFLAG_IPTTL;
				proto->tcpipttl = $3;
			} else if (strcasecmp("minttl", $2) == 0) {
				proto->tcpflags |= TCPFLAG_IPMINTTL;
				proto->tcpipminttl = $3;
			} else {
				yyerror("invalid TCP/IP flag: %s", $2);
				free($2);
				YYERROR;
			}
			free($2);
		}
		;

sslflags_l	: sslflags comma sslflags_l
		| sslflags
		;

sslflags	: SESSION CACHE sslcache	{ proto->cache = $3; }
		| CIPHERS STRING		{
			if (strlcpy(proto->sslciphers, $2,
			    sizeof(proto->sslciphers)) >=
			    sizeof(proto->sslciphers)) {
				yyerror("sslciphers truncated");
				free($2);
				YYERROR;
			}
			free($2);
		}
		| NO flag			{ proto->sslflags &= ~($2); }
		| flag				{ proto->sslflags |= $1; }
		;

flag		: STRING			{
			if (strcmp("sslv2", $1) == 0)
				$$ = SSLFLAG_SSLV2;
			else if (strcmp("sslv3", $1) == 0)
				$$ = SSLFLAG_SSLV3;
			else if (strcmp("tlsv1", $1) == 0)
				$$ = SSLFLAG_TLSV1;
			else {
				yyerror("invalid SSL flag: %s", $1);
				free($1);
				YYERROR;
			}
			free($1);
		}
		;

protonode	: nodetype APPEND STRING TO STRING marked	{
			node.action = NODE_ACTION_APPEND;
			node.key = strdup($5);
			node.value = strdup($3);
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			if (strchr(node.value, '$') != NULL)
				node.flags |= PNFLAG_MACRO;
			free($5);
			free($3);
		}
		| nodetype CHANGE STRING TO STRING marked {
			node.action = NODE_ACTION_CHANGE;
			node.key = strdup($3);
			node.value = strdup($5);
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			if (strchr(node.value, '$') != NULL)
				node.flags |= PNFLAG_MACRO;
			free($5);
			free($3);
		}
		| nodetype REMOVE STRING marked			{
			node.action = NODE_ACTION_REMOVE;
			node.key = strdup($3);
			node.value = NULL;
			if (node.key == NULL)
				fatal("out of memory");
			free($3);
		}
		| nodetype EXPECT STRING FROM STRING marked	{
			node.action = NODE_ACTION_EXPECT;
			node.key = strdup($5);
			node.value = strdup($3);
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($5);
			free($3);
			proto->lateconnect++;
		}
		| nodetype EXPECT STRING marked			{
			node.action = NODE_ACTION_EXPECT;
			node.key = strdup($3);
			node.value = strdup("*");
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($3);
			proto->lateconnect++;
		}
		| nodetype EXPECT digest marked			{
			if (node.type != NODE_TYPE_URL) {
				yyerror("digest not supported for this type");
				free($3.digest);
				YYERROR;
			}
			node.action = NODE_ACTION_EXPECT;
			node.key = strdup($3.digest);
			node.flags |= PNFLAG_LOOKUP_DIGEST($3.type);
			node.value = strdup("*");
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($3.digest);
			proto->lateconnect++;
		}
		| nodetype FILTER STRING FROM STRING marked	{
			node.action = NODE_ACTION_FILTER;
			node.key = strdup($5);
			node.value = strdup($3);
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($5);
			free($3);
			proto->lateconnect++;
		}
		| nodetype FILTER STRING marked			{
			node.action = NODE_ACTION_FILTER;
			node.key = strdup($3);
			node.value = strdup("*");
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($3);
			proto->lateconnect++;
		}
		| nodetype FILTER digest marked			{
			if (node.type != NODE_TYPE_URL) {
				yyerror("digest not supported for this type");
				free($3.digest);
				YYERROR;
			}
			node.action = NODE_ACTION_FILTER;
			node.key = strdup($3.digest);
			node.flags |= PNFLAG_LOOKUP_DIGEST($3.type);
			node.value = strdup("*");
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($3.digest);
			proto->lateconnect++;
		}
		| nodetype HASH STRING marked			{
			node.action = NODE_ACTION_HASH;
			node.key = strdup($3);
			node.value = NULL;
			if (node.key == NULL)
				fatal("out of memory");
			free($3);
			proto->lateconnect++;
		}
		| nodetype LOG STRING marked			{
			node.action = NODE_ACTION_LOG;
			node.key = strdup($3);
			node.value = NULL;
			node.flags |= PNFLAG_LOG;
			if (node.key == NULL)
				fatal("out of memory");
			free($3);
		}
		| nodetype MARK STRING FROM STRING WITH mark	{
			node.action = NODE_ACTION_MARK;
			node.key = strdup($5);
			node.value = strdup($3);
			node.mark = $7;
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($3);
			free($5);
		}
		| nodetype MARK STRING WITH mark		{
			node.action = NODE_ACTION_MARK;
			node.key = strdup($3);
			node.value = strdup("*");
			node.mark = $5;
			if (node.key == NULL || node.value == NULL)
				fatal("out of memory");
			free($3);
		}
		;

marked		: /* empty */
		| MARKED mark			{ node.mark = $2; }
		;

mark		: NUMBER					{
			if ($1 <= 0 || $1 >= (int)USHRT_MAX) {
				yyerror("invalid mark: %d", $1);
				YYERROR;
			}
			$$ = $1;
		}
		;

nodetype	: HEADER			{
			node.type = NODE_TYPE_HEADER;
		}
		| QUERYSTR			{ node.type = NODE_TYPE_QUERY; }
		| COOKIE			{
			node.type = NODE_TYPE_COOKIE;
		}
		| PATH				{
			proto->flags |= F_LOOKUP_PATH;
			node.type = NODE_TYPE_PATH;
		}
		| URL				{ node.type = NODE_TYPE_URL; }
		;

sslcache	: NUMBER			{
			if ($1 < 0) {
				yyerror("invalid sslcache value: %d", $1);
				YYERROR;
			}
			$$ = $1;
		}
		| DISABLE			{ $$ = -2; }
		;

relay		: RELAY STRING	{
			struct relay *r;

			TAILQ_FOREACH(r, conf->sc_relays, rl_entry)
				if (!strcmp(r->rl_conf.name, $2))
					break;
			if (r != NULL) {
				yyerror("relay %s defined twice", $2);
				free($2);
				YYERROR;
			}
			if ((r = calloc(1, sizeof (*r))) == NULL)
				fatal("out of memory");

			if (strlcpy(r->rl_conf.name, $2, sizeof(r->rl_conf.name)) >=
			    sizeof(r->rl_conf.name)) {
				yyerror("relay name truncated");
				free(r);
				YYERROR;
			}
			free($2);
			r->rl_conf.id = ++last_relay_id;
			r->rl_conf.timeout.tv_sec = RELAY_TIMEOUT;
			r->rl_proto = NULL;
			r->rl_conf.proto = EMPTY_ID;
			r->rl_conf.dsttable = EMPTY_ID;
			r->rl_conf.dstmode = RELAY_DSTMODE_DEFAULT;
			r->rl_conf.dstretry = 0;
			if (last_relay_id == INT_MAX) {
				yyerror("too many relays defined");
				free(r);
				YYERROR;
			}
			rlay = r;
		} '{' optnl relayopts_l '}'	{
			if (rlay->rl_conf.ss.ss_family == AF_UNSPEC) {
				yyerror("relay %s has no listener",
				    rlay->rl_conf.name);
				YYERROR;
			}
			if ((rlay->rl_conf.flags & F_NATLOOK) == 0 &&
			    rlay->rl_conf.dstss.ss_family == AF_UNSPEC &&
			    rlay->rl_conf.dsttable == EMPTY_ID) {
				yyerror("relay %s has no target, rdr, "
				    "or table", rlay->rl_conf.name);
				YYERROR;
			}
			if (rlay->rl_conf.proto == EMPTY_ID) {
				rlay->rl_proto = &conf->sc_proto_default;
				rlay->rl_conf.proto = conf->sc_proto_default.id;
			}
			if (relay_load_certfiles(rlay) == -1) {
				yyerror("cannot load certificates for relay %s",
				    rlay->rl_conf.name);
				YYERROR;
			}
			conf->sc_relaycount++;
			SPLAY_INIT(&rlay->rl_sessions);
			TAILQ_INSERT_TAIL(conf->sc_relays, rlay, rl_entry);
			tableport = 0;
			rlay = NULL;
		}
		;

relayopts_l	: relayopts_l relayoptsl nl
		| relayoptsl optnl
		;

relayoptsl	: LISTEN ON STRING port optssl {
			struct addresslist	 al;
			struct address		*h;

			if (rlay->rl_conf.ss.ss_family != AF_UNSPEC) {
				yyerror("relay %s listener already specified",
				    rlay->rl_conf.name);
				YYERROR;
			}

			TAILQ_INIT(&al);
			if (host($3, &al, 1, $4, NULL) <= 0) {
				yyerror("invalid listen ip: %s", $3);
				free($3);
				YYERROR;
			}
			free($3);
			h = TAILQ_FIRST(&al);
			bcopy(&h->ss, &rlay->rl_conf.ss, sizeof(rlay->rl_conf.ss));
			rlay->rl_conf.port = h->port;
			if ($5) {
				rlay->rl_conf.flags |= F_SSL;
				conf->sc_flags |= F_SSL;
			}
			tableport = h->port;
		}
		| forwardmode TO forwardspec interface dstaf	{
			rlay->rl_conf.fwdmode = $1;
			switch ($1) {
			case FWD_NORMAL:
				if ($4 == NULL)
					break;
				yyerror("superfluous interface");
				YYERROR;
			case FWD_ROUTE:
				yyerror("no route for redirections");
				YYERROR;
			case FWD_TRANS:
				if ($4 != NULL)
					break;
				yyerror("missing interface");
				YYERROR;
			}
			if ($4 != NULL) {
				strlcpy(rlay->rl_conf.ifname, $4,
				    sizeof(rlay->rl_conf.ifname));
				free($4);
			}
		}
		| SESSION TIMEOUT NUMBER		{
			if ((rlay->rl_conf.timeout.tv_sec = $3) < 0) {
				yyerror("invalid timeout: %d", $3);
				YYERROR;
			}
		}
		| PROTO STRING			{
			struct protocol *p;

			TAILQ_FOREACH(p, conf->sc_protos, entry)
				if (!strcmp(p->name, $2))
					break;
			if (p == NULL) {
				yyerror("no such protocol: %s", $2);
				free($2);
				YYERROR;
			}
			p->flags |= F_USED;
			rlay->rl_conf.proto = p->id;
			rlay->rl_proto = p;
			free($2);
		}
		| DISABLE		{ rlay->rl_conf.flags |= F_DISABLE; }
		| include
		;

forwardspec	: tablespec	{
			if (rlay->rl_dsttable) {
				yyerror("table already specified");
				purge_table(conf->sc_tables, $1);
				YYERROR;
			}

			rlay->rl_dsttable = $1;
			rlay->rl_dsttable->conf.flags |= F_USED;
			rlay->rl_conf.dsttable = $1->conf.id;
			rlay->rl_conf.dstport = $1->conf.port;
		}
		| STRING port retry {
			struct addresslist	 al;
			struct address		*h;

			if (rlay->rl_conf.dstss.ss_family != AF_UNSPEC) {
				yyerror("relay %s target or redirection already "
				    "specified", rlay->rl_conf.name);
				free($1);
				YYERROR;
			}

			TAILQ_INIT(&al);
			if (host($1, &al, 1, $2, NULL) <= 0) {
				yyerror("invalid listen ip: %s", $1);
				free($1);
				YYERROR;
			}
			free($1);
			h = TAILQ_FIRST(&al);
			bcopy(&h->ss, &rlay->rl_conf.dstss,
			    sizeof(rlay->rl_conf.dstss));
			rlay->rl_conf.dstport = h->port;
			rlay->rl_conf.dstretry = $3;
		}
		| NAT LOOKUP retry		{
			conf->sc_flags |= F_NEEDPF;
			rlay->rl_conf.flags |= F_NATLOOK;
			rlay->rl_conf.dstretry = $3;
		}
		;

dstmode		: /* empty */		{ $$ = RELAY_DSTMODE_DEFAULT; }
		| LOADBALANCE		{ $$ = RELAY_DSTMODE_LOADBALANCE; }
		| ROUNDROBIN		{ $$ = RELAY_DSTMODE_ROUNDROBIN; }
		| HASH			{ $$ = RELAY_DSTMODE_HASH; }
		;

dstaf		: /* empty */		{
			rlay->rl_conf.dstaf.ss_family = AF_UNSPEC;
		}
		| INET			{
			rlay->rl_conf.dstaf.ss_family = AF_INET;
		}
		| INET6	STRING		{
			struct sockaddr_in6	*sin6;

			sin6 = (struct sockaddr_in6 *)&rlay->rl_conf.dstaf;
			if (inet_pton(AF_INET6, $2, &sin6->sin6_addr) == -1) {
				yyerror("invalid ipv6 address %s", $2);
				free($2);
				YYERROR;
			}
			free($2);

			sin6->sin6_family = AF_INET6;
			sin6->sin6_len = sizeof(*sin6);
		}
		;

interface	: /*empty*/		{ $$ = NULL; }
		| INTERFACE STRING	{ $$ = $2; }
		;

host		: STRING retry parent	{
			struct address *a;
			struct addresslist al;

			if (($$ = calloc(1, sizeof(*($$)))) == NULL)
				fatal("out of memory");

			TAILQ_INIT(&al);
			if (host($1, &al, 1, 0, NULL) <= 0) {
				yyerror("invalid host %s", $2);
				free($1);
				free($$);
				YYERROR;
			}
			a = TAILQ_FIRST(&al);
			memcpy(&$$->conf.ss, &a->ss, sizeof($$->conf.ss));
			free(a);

			if (strlcpy($$->conf.name, $1, sizeof($$->conf.name)) >=
			    sizeof($$->conf.name)) {
				yyerror("host name truncated");
				free($1);
				free($$);
				YYERROR;
			}
			free($1);
			$$->conf.id = 0; /* will be set later */
			$$->conf.retry = $2;
			$$->conf.parentid = $3;
			SLIST_INIT(&$$->children);
		}
		;

retry		: /* nothing */		{ $$ = 0; }
		| RETRY NUMBER		{
			if (($$ = $2) < 0) {
				yyerror("invalid retry value: %d\n", $2);
				YYERROR;
			}
		}
		;

parent		: /* nothing */		{ $$ = 0; }
		| PARENT NUMBER		{
			if (($$ = $2) < 0) {
				yyerror("invalid parent value: %d\n", $2);
				YYERROR;
			}
		}
		;

timeout		: NUMBER
		{
			if ($1 < 0) {
				yyerror("invalid timeout: %d\n", $1);
				YYERROR;
			}
			$$.tv_sec = $1 / 1000;
			$$.tv_usec = ($1 % 1000) * 1000;
		}
		;

log		: /* empty */		{ $$ = 0; }
		| LOG			{ $$ = 1; }
		;

comma		: ','
		| nl
		| /* empty */
		;

optnl		: '\n' optnl
		|
		;

nl		: '\n' optnl
		;

%%

struct keywords {
	const char	*k_name;
	int		 k_val;
};

int
yyerror(const char *fmt, ...)
{
	va_list		 ap;

	file->errors++;
	va_start(ap, fmt);
	fprintf(stderr, "%s:%d: ", file->name, yylval.lineno);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	return (0);
}

int
kw_cmp(const void *k, const void *e)
{
	return (strcmp(k, ((const struct keywords *)e)->k_name));
}

int
lookup(char *s)
{
	/* this has to be sorted always */
	static const struct keywords keywords[] = {
		{ "all",		ALL },
		{ "append",		APPEND },
		{ "backlog",		BACKLOG },
		{ "backup",		BACKUP },
		{ "buffer",		BUFFER },
		{ "cache",		CACHE },
		{ "change",		CHANGE },
		{ "check",		CHECK },
		{ "ciphers",		CIPHERS },
		{ "code",		CODE },
		{ "cookie",		COOKIE },
		{ "demote",		DEMOTE },
		{ "digest",		DIGEST },
		{ "disable",		DISABLE },
		{ "error",		ERROR },
		{ "expect",		EXPECT },
		{ "external",		EXTERNAL },
		{ "filter",		FILTER },
		{ "forward",		FORWARD },
		{ "from",		FROM },
		{ "hash",		HASH },
		{ "header",		HEADER },
		{ "host",		HOST },
		{ "icmp",		ICMP },
		{ "include",		INCLUDE },
		{ "inet",		INET },
		{ "inet6",		INET6 },
		{ "interface",		INTERFACE },
		{ "interval",		INTERVAL },
		{ "ip",			IP },
		{ "label",		LABEL },
		{ "listen",		LISTEN },
		{ "loadbalance",	LOADBALANCE },
		{ "log",		LOG },
		{ "lookup",		LOOKUP },
		{ "mark",		MARK },
		{ "marked",		MARKED },
		{ "mode",		MODE },
		{ "nat",		NAT },
		{ "no",			NO },
		{ "nodelay",		NODELAY },
		{ "nothing",		NOTHING },
		{ "on",			ON },
		{ "parent",		PARENT },
		{ "path",		PATH },
		{ "port",		PORT },
		{ "prefork",		PREFORK },
		{ "protocol",		PROTO },
		{ "query",		QUERYSTR },
		{ "real",		REAL },
		{ "redirect",		REDIRECT },
		{ "relay",		RELAY },
		{ "remove",		REMOVE },
		{ "request",		REQUEST },
		{ "response",		RESPONSE },
		{ "retry",		RETRY },
		{ "return",		RETURN },
		{ "roundrobin",		ROUNDROBIN },
		{ "route",		ROUTE },
		{ "sack",		SACK },
		{ "script",		SCRIPT },
		{ "send",		SEND },
		{ "session",		SESSION },
		{ "socket",		SOCKET },
		{ "ssl",		SSL },
		{ "sticky-address",	STICKYADDR },
		{ "style",		STYLE },
		{ "table",		TABLE },
		{ "tag",		TAG },
		{ "tcp",		TCP },
		{ "timeout",		TIMEOUT },
		{ "to",			TO },
		{ "transparent",	TRANSPARENT },
		{ "trap",		TRAP },
		{ "updates",		UPDATES },
		{ "url",		URL },
		{ "virtual",		VIRTUAL },
		{ "with",		WITH }
	};
	const struct keywords	*p;

	p = bsearch(s, keywords, sizeof(keywords)/sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	if (p)
		return (p->k_val);
	else
		return (STRING);
}

#define MAXPUSHBACK	128

char	*parsebuf;
int	 parseindex;
char	 pushback_buffer[MAXPUSHBACK];
int	 pushback_index = 0;

int
lgetc(int quotec)
{
	int		c, next;

	if (parsebuf) {
		/* Read character from the parsebuffer instead of input. */
		if (parseindex >= 0) {
			c = parsebuf[parseindex++];
			if (c != '\0')
				return (c);
			parsebuf = NULL;
		} else
			parseindex++;
	}

	if (pushback_index)
		return (pushback_buffer[--pushback_index]);

	if (quotec) {
		if ((c = getc(file->stream)) == EOF) {
			yyerror("reached end of file while parsing "
			    "quoted string");
			if (file == topfile || popfile() == EOF)
				return (EOF);
			return (quotec);
		}
		return (c);
	}

	while ((c = getc(file->stream)) == '\\') {
		next = getc(file->stream);
		if (next != '\n') {
			c = next;
			break;
		}
		yylval.lineno = file->lineno;
		file->lineno++;
	}

	while (c == EOF) {
		if (file == topfile || popfile() == EOF)
			return (EOF);
		c = getc(file->stream);
	}
	return (c);
}

int
lungetc(int c)
{
	if (c == EOF)
		return (EOF);
	if (parsebuf) {
		parseindex--;
		if (parseindex >= 0)
			return (c);
	}
	if (pushback_index < MAXPUSHBACK-1)
		return (pushback_buffer[pushback_index++] = c);
	else
		return (EOF);
}

int
findeol(void)
{
	int	c;

	parsebuf = NULL;
	pushback_index = 0;

	/* skip to either EOF or the first real EOL */
	while (1) {
		c = lgetc(0);
		if (c == '\n') {
			file->lineno++;
			break;
		}
		if (c == EOF)
			break;
	}
	return (ERROR);
}

int
yylex(void)
{
	char	 buf[8096];
	char	*p, *val;
	int	 quotec, next, c;
	int	 token;

top:
	p = buf;
	while ((c = lgetc(0)) == ' ' || c == '\t')
		; /* nothing */

	yylval.lineno = file->lineno;
	if (c == '#')
		while ((c = lgetc(0)) != '\n' && c != EOF)
			; /* nothing */
	if (c == '$' && parsebuf == NULL) {
		while (1) {
			if ((c = lgetc(0)) == EOF)
				return (0);

			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			if (isalnum(c) || c == '_') {
				*p++ = (char)c;
				continue;
			}
			*p = '\0';
			lungetc(c);
			break;
		}
		val = symget(buf);
		if (val == NULL) {
			yyerror("macro '%s' not defined", buf);
			return (findeol());
		}
		parsebuf = val;
		parseindex = 0;
		goto top;
	}

	switch (c) {
	case '\'':
	case '"':
		quotec = c;
		while (1) {
			if ((c = lgetc(quotec)) == EOF)
				return (0);
			if (c == '\n') {
				file->lineno++;
				continue;
			} else if (c == '\\') {
				if ((next = lgetc(quotec)) == EOF)
					return (0);
				if (next == quotec || c == ' ' || c == '\t')
					c = next;
				else if (next == '\n')
					continue;
				else
					lungetc(next);
			} else if (c == quotec) {
				*p = '\0';
				break;
			}
			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			*p++ = (char)c;
		}
		yylval.v.string = strdup(buf);
		if (yylval.v.string == NULL)
			err(1, "yylex: strdup");
		return (STRING);
	}

#define allowed_to_end_number(x) \
	(isspace(x) || x == ')' || x ==',' || x == '/' || x == '}' || x == '=')

	if (c == '-' || isdigit(c)) {
		do {
			*p++ = c;
			if ((unsigned)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && isdigit(c));
		lungetc(c);
		if (p == buf + 1 && buf[0] == '-')
			goto nodigits;
		if (c == EOF || allowed_to_end_number(c)) {
			const char *errstr = NULL;

			*p = '\0';
			yylval.v.number = strtonum(buf, LLONG_MIN,
			    LLONG_MAX, &errstr);
			if (errstr) {
				yyerror("\"%s\" invalid number: %s",
				    buf, errstr);
				return (findeol());
			}
			return (NUMBER);
		} else {
nodigits:
			while (p > buf + 1)
				lungetc(*--p);
			c = *--p;
			if (c == '-')
				return (c);
		}
	}

#define allowed_in_string(x) \
	(isalnum(x) || (ispunct(x) && x != '(' && x != ')' && \
	x != '{' && x != '}' && x != '<' && x != '>' && \
	x != '!' && x != '=' && x != '#' && \
	x != ','))

	if (isalnum(c) || c == ':' || c == '_') {
		do {
			*p++ = c;
			if ((unsigned)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && (allowed_in_string(c)));
		lungetc(c);
		*p = '\0';
		if ((token = lookup(buf)) == STRING)
			if ((yylval.v.string = strdup(buf)) == NULL)
				err(1, "yylex: strdup");
		return (token);
	}
	if (c == '\n') {
		yylval.lineno = file->lineno;
		file->lineno++;
	}
	if (c == EOF)
		return (0);
	return (c);
}

int
check_file_secrecy(int fd, const char *fname)
{
	struct stat	st;

	if (fstat(fd, &st)) {
		log_warn("cannot stat %s", fname);
		return (-1);
	}
	if (st.st_uid != 0 && st.st_uid != getuid()) {
		log_warnx("%s: owner not root or current user", fname);
		return (-1);
	}
	if (st.st_mode & (S_IRWXG | S_IRWXO)) {
		log_warnx("%s: group/world readable/writeable", fname);
		return (-1);
	}
	return (0);
}

struct file *
pushfile(const char *name, int secret)
{
	struct file	*nfile;

	if ((nfile = calloc(1, sizeof(struct file))) == NULL ||
	    (nfile->name = strdup(name)) == NULL) {
		log_warn("malloc");
		return (NULL);
	}
	if ((nfile->stream = fopen(nfile->name, "r")) == NULL) {
		log_warn("%s", nfile->name);
		free(nfile->name);
		free(nfile);
		return (NULL);
	} else if (secret &&
	    check_file_secrecy(fileno(nfile->stream), nfile->name)) {
		fclose(nfile->stream);
		free(nfile->name);
		free(nfile);
		return (NULL);
	}
	nfile->lineno = 1;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

int
popfile(void)
{
	struct file	*prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != NULL)
		prev->errors += file->errors;

	TAILQ_REMOVE(&files, file, entry);
	fclose(file->stream);
	free(file->name);
	free(file);
	file = prev;
	return (file ? 0 : EOF);
}

struct relayd *
parse_config(const char *filename, int opts)
{
	struct sym	*sym, *next;
	struct table	*nexttb;
	struct host	*h, *ph;

	if ((conf = calloc(1, sizeof(*conf))) == NULL ||
	    (conf->sc_tables = calloc(1, sizeof(*conf->sc_tables))) == NULL ||
	    (conf->sc_relays = calloc(1, sizeof(*conf->sc_relays))) == NULL ||
	    (conf->sc_protos = calloc(1, sizeof(*conf->sc_protos))) == NULL ||
	    (conf->sc_rdrs = calloc(1, sizeof(*conf->sc_rdrs))) == NULL) {
		log_warn("cannot allocate memory");
		return (NULL);
	}

	errors = 0;
	last_host_id = last_table_id = last_rdr_id = last_proto_id =
	    last_relay_id = 0;

	rdr = NULL;
	table = NULL;
	rlay = NULL;
	proto = NULL;

	TAILQ_INIT(conf->sc_rdrs);
	TAILQ_INIT(conf->sc_tables);
	TAILQ_INIT(conf->sc_protos);
	TAILQ_INIT(conf->sc_relays);

	memset(&conf->sc_empty_table, 0, sizeof(conf->sc_empty_table));
	conf->sc_empty_table.conf.id = EMPTY_TABLE;
	conf->sc_empty_table.conf.flags |= F_DISABLE;
	(void)strlcpy(conf->sc_empty_table.conf.name, "empty",
	    sizeof(conf->sc_empty_table.conf.name));

	bzero(&conf->sc_proto_default, sizeof(conf->sc_proto_default));
	conf->sc_proto_default.flags = F_USED;
	conf->sc_proto_default.cache = RELAY_CACHESIZE;
	conf->sc_proto_default.type = RELAY_PROTO_TCP;
	(void)strlcpy(conf->sc_proto_default.name, "default",
	    sizeof(conf->sc_proto_default.name));
	RB_INIT(&conf->sc_proto_default.request_tree);
	RB_INIT(&conf->sc_proto_default.response_tree);

	conf->sc_timeout.tv_sec = CHECK_TIMEOUT / 1000;
	conf->sc_timeout.tv_usec = (CHECK_TIMEOUT % 1000) * 1000;
	conf->sc_interval.tv_sec = CHECK_INTERVAL;
	conf->sc_interval.tv_usec = 0;
	conf->sc_prefork_relay = RELAY_NUMPROC;
	conf->sc_statinterval.tv_sec = RELAY_STATINTERVAL;
	conf->sc_opts = opts;
	conf->sc_confpath = filename;

	if ((file = pushfile(filename, 0)) == NULL) {
		free(conf);
		return (NULL);
	}
	topfile = file;
	setservent(1);

	yyparse();
	errors = file->errors;
	popfile();

	endservent();

	/* Free macros and check which have not been used. */
	for (sym = TAILQ_FIRST(&symhead); sym != NULL; sym = next) {
		next = TAILQ_NEXT(sym, entry);
		if ((conf->sc_opts & RELAYD_OPT_VERBOSE) && !sym->used)
			fprintf(stderr, "warning: macro '%s' not "
			    "used\n", sym->nam);
		if (!sym->persist) {
			free(sym->nam);
			free(sym->val);
			TAILQ_REMOVE(&symhead, sym, entry);
			free(sym);
		}
	}

	if (TAILQ_EMPTY(conf->sc_rdrs) && TAILQ_EMPTY(conf->sc_relays)) {
		log_warnx("no redirections, nothing to do");
		errors++;
	}

	if (TAILQ_EMPTY(conf->sc_relays))
		conf->sc_prefork_relay = 0;

	if (timercmp(&conf->sc_timeout, &conf->sc_interval, >=)) {
		log_warnx("global timeout exceeds interval");
		errors++;
	}

	/* Verify that every table is used */
	for (table = TAILQ_FIRST(conf->sc_tables); table != NULL;
	     table = nexttb) {
		nexttb = TAILQ_NEXT(table, entry);
		if (table->conf.port == 0) {
			TAILQ_REMOVE(conf->sc_tables, table, entry);
			while ((h = TAILQ_FIRST(&table->hosts)) != NULL) {
				TAILQ_REMOVE(&table->hosts, h, entry);
				free(h);
			}
			if (table->sendbuf != NULL)
				free(table->sendbuf);
			free(table);
			continue;
		}

		TAILQ_FOREACH(h, &table->hosts, entry) {
			if (h->conf.parentid) {
				ph = host_find(conf, h->conf.parentid);

				/* Validate the parent id */
				if (h->conf.id == h->conf.parentid ||
				    ph == NULL || ph->conf.parentid)
					ph = NULL;

				if (ph == NULL) {
					log_warnx("host parent id %d invalid",
					    h->conf.parentid);
					errors++;
				} else
					SLIST_INSERT_HEAD(&ph->children,
					    h, child);
			}
		}

		if (!(table->conf.flags & F_USED)) {
			log_warnx("unused table: %s", table->conf.name);
			errors++;
		}
		if (timercmp(&table->conf.timeout, &conf->sc_interval, >=)) {
			log_warnx("table timeout exceeds interval: %s",
			    table->conf.name);
			errors++;
		}
	}

	/* Verify that every non-default protocol is used */
	TAILQ_FOREACH(proto, conf->sc_protos, entry) {
		if (!(proto->flags & F_USED)) {
			log_warnx("unused protocol: %s", proto->name);
		}
	}

	if (errors) {
		free(conf);
		return (NULL);
	}

	return (conf);
}

int
symset(const char *nam, const char *val, int persist)
{
	struct sym	*sym;

	for (sym = TAILQ_FIRST(&symhead); sym && strcmp(nam, sym->nam);
	    sym = TAILQ_NEXT(sym, entry))
		;	/* nothing */

	if (sym != NULL) {
		if (sym->persist == 1)
			return (0);
		else {
			free(sym->nam);
			free(sym->val);
			TAILQ_REMOVE(&symhead, sym, entry);
			free(sym);
		}
	}
	if ((sym = calloc(1, sizeof(*sym))) == NULL)
		return (-1);

	sym->nam = strdup(nam);
	if (sym->nam == NULL) {
		free(sym);
		return (-1);
	}
	sym->val = strdup(val);
	if (sym->val == NULL) {
		free(sym->nam);
		free(sym);
		return (-1);
	}
	sym->used = 0;
	sym->persist = persist;
	TAILQ_INSERT_TAIL(&symhead, sym, entry);
	return (0);
}

int
cmdline_symset(char *s)
{
	char	*sym, *val;
	int	ret;
	size_t	len;

	if ((val = strrchr(s, '=')) == NULL)
		return (-1);

	len = strlen(s) - strlen(val) + 1;
	if ((sym = malloc(len)) == NULL)
		errx(1, "cmdline_symset: malloc");

	(void)strlcpy(sym, s, len);

	ret = symset(sym, val + 1, 1);
	free(sym);

	return (ret);
}

char *
symget(const char *nam)
{
	struct sym	*sym;

	TAILQ_FOREACH(sym, &symhead, entry)
		if (strcmp(nam, sym->nam) == 0) {
			sym->used = 1;
			return (sym->val);
		}
	return (NULL);
}

struct address *
host_v4(const char *s)
{
	struct in_addr		 ina;
	struct sockaddr_in	*sain;
	struct address		*h;

	bzero(&ina, sizeof(ina));
	if (inet_pton(AF_INET, s, &ina) != 1)
		return (NULL);

	if ((h = calloc(1, sizeof(*h))) == NULL)
		fatal(NULL);
	sain = (struct sockaddr_in *)&h->ss;
	sain->sin_len = sizeof(struct sockaddr_in);
	sain->sin_family = AF_INET;
	sain->sin_addr.s_addr = ina.s_addr;

	return (h);
}

struct address *
host_v6(const char *s)
{
	struct addrinfo		 hints, *res;
	struct sockaddr_in6	*sa_in6;
	struct address		*h = NULL;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM; /* dummy */
	hints.ai_flags = AI_NUMERICHOST;
	if (getaddrinfo(s, "0", &hints, &res) == 0) {
		if ((h = calloc(1, sizeof(*h))) == NULL)
			fatal(NULL);
		sa_in6 = (struct sockaddr_in6 *)&h->ss;
		sa_in6->sin6_len = sizeof(struct sockaddr_in6);
		sa_in6->sin6_family = AF_INET6;
		memcpy(&sa_in6->sin6_addr,
		    &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr,
		    sizeof(sa_in6->sin6_addr));
		sa_in6->sin6_scope_id =
		    ((struct sockaddr_in6 *)res->ai_addr)->sin6_scope_id;

		freeaddrinfo(res);
	}

	return (h);
}

int
host_dns(const char *s, struct addresslist *al, int max,
	 in_port_t port, const char *ifname)
{
	struct addrinfo		 hints, *res0, *res;
	int			 error, cnt = 0;
	struct sockaddr_in	*sain;
	struct sockaddr_in6	*sin6;
	struct address		*h;

	bzero(&hints, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM; /* DUMMY */
	error = getaddrinfo(s, NULL, &hints, &res0);
	if (error == EAI_AGAIN || error == EAI_NODATA || error == EAI_NONAME)
		return (0);
	if (error) {
		log_warnx("host_dns: could not parse \"%s\": %s", s,
		    gai_strerror(error));
		return (-1);
	}

	for (res = res0; res && cnt < max; res = res->ai_next) {
		if (res->ai_family != AF_INET &&
		    res->ai_family != AF_INET6)
			continue;
		if ((h = calloc(1, sizeof(*h))) == NULL)
			fatal(NULL);

		h->port = port;
		if (ifname != NULL) {
			if (strlcpy(h->ifname, ifname, sizeof(h->ifname)) >=
			    sizeof(h->ifname))
				log_warnx("host_dns: interface name truncated");
			freeaddrinfo(res0);
			return (-1);
		}
		h->ss.ss_family = res->ai_family;
		if (res->ai_family == AF_INET) {
			sain = (struct sockaddr_in *)&h->ss;
			sain->sin_len = sizeof(struct sockaddr_in);
			sain->sin_addr.s_addr = ((struct sockaddr_in *)
			    res->ai_addr)->sin_addr.s_addr;
		} else {
			sin6 = (struct sockaddr_in6 *)&h->ss;
			sin6->sin6_len = sizeof(struct sockaddr_in6);
			memcpy(&sin6->sin6_addr, &((struct sockaddr_in6 *)
			    res->ai_addr)->sin6_addr, sizeof(struct in6_addr));
		}

		TAILQ_INSERT_HEAD(al, h, entry);
		cnt++;
	}
	if (cnt == max && res) {
		log_warnx("host_dns: %s resolves to more than %d hosts",
		    s, max);
	}
	freeaddrinfo(res0);
	return (cnt);
}

int
host(const char *s, struct addresslist *al, int max,
    in_port_t port, const char *ifname)
{
	struct address *h;

	h = host_v4(s);

	/* IPv6 address? */
	if (h == NULL)
		h = host_v6(s);

	if (h != NULL) {
		h->port = port;
		if (ifname != NULL) {
			if (strlcpy(h->ifname, ifname, sizeof(h->ifname)) >=
			    sizeof(h->ifname)) {
				log_warnx("host: interface name truncated");
				return (-1);
			}
		}

		TAILQ_INSERT_HEAD(al, h, entry);
		return (1);
	}

	return (host_dns(s, al, max, port, ifname));
}

struct table *
table_inherit(struct table *tb)
{
	char		pname[TABLE_NAME_SIZE + 6];
	struct host	*h, *dsth;
	struct table	*dsttb, *oldtb;

	/* Get the table or table template */
	if ((dsttb = table_findbyname(conf, tb->conf.name)) == NULL) {
		yyerror("unknown table %s", tb->conf.name);
		purge_table(NULL, tb);
		return (NULL);
	}
	if (dsttb->conf.port != 0)
		fatal("invalid table");	/* should not happen */

	if (tb->conf.port == 0) {
		yyerror("invalid port");
		purge_table(NULL, tb);
		return (NULL);
	}

	/* Check if a matching table already exists */
	if (snprintf(pname, sizeof(pname), "%s:%u",
	    tb->conf.name, ntohs(tb->conf.port)) >= (int)sizeof(pname)) {
		yyerror("invalid table name");
		return (NULL);
	}
	(void)strlcpy(tb->conf.name, pname, sizeof(tb->conf.name));
	if ((oldtb = table_findbyconf(conf, tb)) != NULL)
		return (oldtb);

	/* Create a new table */
	tb->conf.id = ++last_table_id;
	if (last_table_id == INT_MAX) {
		yyerror("too many tables defined");
		purge_table(NULL, tb);
		return (NULL);
	}
	tb->conf.flags |= dsttb->conf.flags;

	/* Inherit global table options */
	bcopy(&dsttb->conf.timeout, &tb->conf.timeout, sizeof(struct timeval));
	tb->conf.skip_cnt = dsttb->conf.skip_cnt;
	strlcpy(tb->conf.demote_group, dsttb->conf.demote_group,
	    sizeof(tb->conf.demote_group));

	/* Copy the associated hosts */
	TAILQ_INIT(&tb->hosts);
	TAILQ_FOREACH(dsth, &dsttb->hosts, entry) {
		if ((h = (struct host *)
		    calloc(1, sizeof (*h))) == NULL)
			fatal("out of memory");
		bcopy(dsth, h, sizeof(*h));
		h->conf.id = ++last_host_id;
		if (last_host_id == INT_MAX) {
			yyerror("too many hosts defined");
			purge_table(NULL, tb);
			return (NULL);
		}
		h->conf.tableid = tb->conf.id;
		h->tablename = tb->conf.name;
		SLIST_INIT(&h->children);
		TAILQ_INSERT_TAIL(&tb->hosts, h, entry);
	}

	conf->sc_tablecount++;
	TAILQ_INSERT_TAIL(conf->sc_tables, tb, entry);

	return (tb);
}
