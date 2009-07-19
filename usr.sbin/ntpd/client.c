/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 2004 Alexander Guy <alexander.guy@andern.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <errno.h>
#include <md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ntpd.h"

int	client_update(struct ntp_peer *);
void	set_deadline(struct ntp_peer *, time_t);

void
set_next(struct ntp_peer *p, time_t t)
{
	p->next = getmonotime() + t;
	p->deadline = 0;
}

void
set_deadline(struct ntp_peer *p, time_t t)
{
	p->deadline = getmonotime() + t;
	p->next = 0;
}

int
client_peer_init(struct ntp_peer *p)
{
	if ((p->query = calloc(1, sizeof(struct ntp_query))) == NULL)
		fatal("client_peer_init calloc");
	p->query->fd = -1;
	p->query->msg.status = MODE_CLIENT | (NTP_VERSION << 3);
	p->state = STATE_NONE;
	p->shift = 0;
	p->trustlevel = TRUSTLEVEL_PATHETIC;
	p->lasterror = 0;
	p->senderrors = 0;

	return (client_addr_init(p));
}

int
client_addr_init(struct ntp_peer *p)
{
	struct sockaddr_in	*sa_in;
	struct sockaddr_in6	*sa_in6;
	struct ntp_addr		*h;

	for (h = p->addr; h != NULL; h = h->next) {
		switch (h->ss.ss_family) {
		case AF_INET:
			sa_in = (struct sockaddr_in *)&h->ss;
			if (ntohs(sa_in->sin_port) == 0)
				sa_in->sin_port = htons(123);
			p->state = STATE_DNS_DONE;
			break;
		case AF_INET6:
			sa_in6 = (struct sockaddr_in6 *)&h->ss;
			if (ntohs(sa_in6->sin6_port) == 0)
				sa_in6->sin6_port = htons(123);
			p->state = STATE_DNS_DONE;
			break;
		default:
			fatalx("king bula sez: wrong AF in client_addr_init");
			/* not reached */
		}
	}

	p->query->fd = -1;
	set_next(p, 0);

	return (0);
}

int
client_nextaddr(struct ntp_peer *p)
{
	if (p->query->fd != -1) {
		close(p->query->fd);
		p->query->fd = -1;
	}

	if (p->state == STATE_DNS_INPROGRESS)
		return (-1);

	if (p->addr_head.a == NULL) {
		priv_host_dns(p->addr_head.name, p->id);
		p->state = STATE_DNS_INPROGRESS;
		return (-1);
	}

	if ((p->addr = p->addr->next) == NULL)
		p->addr = p->addr_head.a;

	p->shift = 0;
	p->trustlevel = TRUSTLEVEL_PATHETIC;

	return (0);
}

int
client_query(struct ntp_peer *p)
{
	int	val;

	if (p->addr == NULL && client_nextaddr(p) == -1) {
		set_next(p, MAX(SETTIME_TIMEOUT,
		    scale_interval(INTERVAL_QUERY_AGGRESSIVE)));
		return (0);
	}

	if (p->state < STATE_DNS_DONE || p->addr == NULL)
		return (-1);

	if (p->query->fd == -1) {
		struct sockaddr *sa = (struct sockaddr *)&p->addr->ss;

		if ((p->query->fd = socket(p->addr->ss.ss_family, SOCK_DGRAM,
		    0)) == -1)
			fatal("client_query socket");
		if (connect(p->query->fd, sa, SA_LEN(sa)) == -1) {
			if (errno == ECONNREFUSED || errno == ENETUNREACH ||
			    errno == EHOSTUNREACH || errno == EADDRNOTAVAIL) {
				client_nextaddr(p);
				set_next(p, MAX(SETTIME_TIMEOUT,
				    scale_interval(INTERVAL_QUERY_AGGRESSIVE)));
				return (-1);
			} else
				fatal("client_query connect");
		}
		val = IPTOS_LOWDELAY;
		if (p->addr->ss.ss_family == AF_INET && setsockopt(p->query->fd,
		    IPPROTO_IP, IP_TOS, &val, sizeof(val)) == -1)
			log_warn("setsockopt IPTOS_LOWDELAY");
		val = 1;
		if (setsockopt(p->query->fd, SOL_SOCKET, SO_TIMESTAMP,
		    &val, sizeof(val)) < 0)
			fatal("setsockopt SO_TIMESTAMP");
	}

	/*
	 * Send out a random 64-bit number as our transmit time.  The NTP
	 * server will copy said number into the originate field on the
	 * response that it sends us.  This is totally legal per the SNTP spec.
	 *
	 * The impact of this is two fold: we no longer send out the current
	 * system time for the world to see (which may aid an attacker), and
	 * it gives us a (not very secure) way of knowing that we're not
	 * getting spoofed by an attacker that can't capture our traffic
	 * but can spoof packets from the NTP server we're communicating with.
	 *
	 * Save the real transmit timestamp locally.
	 */

	p->query->msg.xmttime.int_partl = arc4random();
	p->query->msg.xmttime.fractionl = arc4random();
	p->query->xmttime = gettime_corrected();

	if (ntp_sendmsg(p->query->fd, NULL, &p->query->msg,
	    NTP_MSGSIZE_NOAUTH, 0) == -1) {
		p->senderrors++;
		set_next(p, INTERVAL_QUERY_PATHETIC);
		p->trustlevel = TRUSTLEVEL_PATHETIC;
		return (-1);
	}

	p->senderrors = 0;
	p->state = STATE_QUERY_SENT;
	set_deadline(p, QUERYTIME_MAX);

	return (0);
}

int
client_dispatch(struct ntp_peer *p, u_int8_t settime)
{
	extern int debug; /* from log.c */
	struct sockaddr_storage fsa;
	struct ntp_msg	msg;
	struct timeval	tv1, tv2;
	char		buf[NTP_MSGSIZE];
	int64_t		T1, T2, T3, T4;
	struct ntp_offset *reply;
	time_t		interval;

	if (ntp_recvmsg(p->query->fd, (struct sockaddr *)&fsa,
	    buf, &msg, &T4) < 0) {
		set_next(p, error_interval());
		return (0);
	}

	if (msg.orgtime.int_partl != p->query->msg.xmttime.int_partl ||
	    msg.orgtime.fractionl != p->query->msg.xmttime.fractionl)
		return (0);

	if ((msg.status & LI_ALARM) == LI_ALARM || msg.stratum == 0 ||
	    msg.stratum > NTP_MAXSTRATUM) {
		char s[16];

		if ((msg.status & LI_ALARM) == LI_ALARM) {
			strlcpy(s, "alarm", sizeof(s));
		} else if (msg.stratum == 0) {
			/* Kiss-o'-Death (KoD) packet */
			strlcpy(s, "KoD", sizeof(s));
		} else if (msg.stratum > NTP_MAXSTRATUM) {
			snprintf(s, sizeof(s), "stratum %d", msg.stratum);
		}
		interval = error_interval();
		set_next(p, interval);
		log_info("reply from %s: not synced (%s), next query %ds",
		    log_sockaddr((struct sockaddr *)&p->addr->ss), s,
			interval);
		return (0);
	}

	/*
	 * From RFC 2030 (with a correction to the delay math):
	 *
	 *     Timestamp Name          ID   When Generated
	 *     ------------------------------------------------------------
	 *     Originate Timestamp     T1   time request sent by client
	 *     Receive Timestamp       T2   time request received by server
	 *     Transmit Timestamp      T3   time reply sent by server
	 *     Destination Timestamp   T4   time reply received by client
	 *
	 *  The roundtrip delay d and local clock offset t are defined as
	 *
	 *    d = (T4 - T1) - (T3 - T2)     t = ((T2 - T1) + (T3 - T4)) / 2.
	 */

	T1 = p->query->xmttime;
	T2 = lfxt2int64(&msg.rectime);
	T3 = lfxt2int64(&msg.xmttime);

	/*
	 * XXX workaround: time_t / tv_sec must never wrap.
	 * around 2020 we will need a solution (64bit time_t / tv_sec).
	 * consider every answer with a timestamp beyond january 2030 bogus.
	 */
	if (T2 > JAN_2030 || T3 > JAN_2030) {
		set_next(p, error_interval());
		return (0);
	}

	if (T4 <= T1) {
		interval = error_interval();
		set_next(p, interval);
		log_info("local clock is not monotonic");
		return (0);
	}

	reply = &p->reply[p->shift];

	reply->offset = ((T2 - T1) + (T3 - T4)) / 2;
	reply->delay = (T4 - T1) - (T3 - T2);
	if (reply->delay < 0) {
		char *s = "";
		int64_t val = reply->delay;
		if (val < 0) {
			s = "-";
			val = -val;
		}
		int642timeval(val, &tv2);
		interval = error_interval();
		set_next(p, interval);
		log_info("reply from %s: negative delay %s%ld.%06lus "
		    "next query %ds",
		    log_sockaddr((struct sockaddr *)&p->addr->ss),
		    s, tv2.tv_sec, tv2.tv_usec, interval);
		return (0);
	}

	p->shift = (p->shift + 1) % OFFSET_ARRAY_SIZE;

	reply->error = (T2 - T1) - (T3 - T4);
	reply->rcvd = getmonotime();
	reply->good = 1;

	reply->status.leap = msg.status & LIMASK;
	reply->status.precision = msg.precision;
	reply->status.refid = ntohl(msg.refid);
	reply->status.poll = msg.ppoll;
	reply->status.stratum = msg.stratum;
	reply->status.rootdelay = sfxt2int64(&msg.rootdelay);
	reply->status.rootdispersion = sfxt2int64(&msg.dispersion);
	reply->status.reftime = lfxt2int64(&msg.reftime);

	if (p->addr->ss.ss_family == AF_INET) {
		reply->status.send_refid =
		    ((struct sockaddr_in *)&p->addr->ss)->sin_addr.s_addr;
	} else if (p->addr->ss.ss_family == AF_INET6) {
		MD5_CTX		context;
		u_int8_t	digest[MD5_DIGEST_LENGTH];

		MD5Init(&context);
		MD5Update(&context, ((struct sockaddr_in6 *)&p->addr->ss)->
		    sin6_addr.s6_addr, sizeof(struct in6_addr));
		MD5Final(digest, &context);
		memcpy((char *)&reply->status.send_refid, digest,
		    sizeof(u_int32_t));
	} else
		reply->status.send_refid = msg.xmttime.fractionl;

	if (p->trustlevel < TRUSTLEVEL_PATHETIC)
		interval = scale_interval(INTERVAL_QUERY_PATHETIC);
	else if (p->trustlevel < TRUSTLEVEL_AGGRESSIVE)
		interval = scale_interval(INTERVAL_QUERY_AGGRESSIVE);
	else
		interval = scale_interval(INTERVAL_QUERY_NORMAL);

	set_next(p, interval);
	p->state = STATE_REPLY_RECEIVED;

	/* every received reply which we do not discard increases trust */
	if (p->trustlevel < TRUSTLEVEL_MAX) {
		if (p->trustlevel < TRUSTLEVEL_BADPEER &&
		    p->trustlevel + 1 >= TRUSTLEVEL_BADPEER)
			log_info("peer %s is now valid",
			    log_sockaddr((struct sockaddr *)&p->addr->ss));
		p->trustlevel++;
	}

	if (debug) {
		char *s = "";
		int64_t val = reply->offset;
		if (val < 0) {
			s = "-";
			val = -val;
		}
		int642timeval(val, &tv1);
		int642timeval(reply->delay, &tv2);
		log_debug("reply from %s: "
		    "offset %s%ld.%06lus delay %ld.%06lus, next query %ds",
		    log_sockaddr((struct sockaddr *)&p->addr->ss),
		    s, tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec,
		    interval);
	}

	client_update(p);
	if (settime)
		priv_settime(reply->offset);

	return (0);
}

int
client_update(struct ntp_peer *p)
{
	struct ntp_offset	*reply, *best, *last;
	int	good;

	/*
	 * clock filter
	 * find the offset which arrived with the lowest delay
	 * and lowest stratum; use that as the peer update.
	 * invalidate it and all older ones
	 */

	for (good = 0, best = NULL, reply = &p->reply[0],
	    last = &p->reply[OFFSET_ARRAY_SIZE]; reply < last; reply++) {
		if (!reply->good)
			continue;
		good++;
		if (!best)
			best = reply;
		if (reply->status.stratum <= best->status.stratum &&
		    reply->delay < best->delay)
			best = reply;
	}

	if (good < OFFSET_ARRAY_SIZE)
		return (-1);

	memcpy(&p->update, best, sizeof(p->update));
	if (priv_adjtime() == 0)
		for (reply = &p->reply[0], last = &p->reply[OFFSET_ARRAY_SIZE];
		    reply < last; reply++)
			if (reply->rcvd <= best->rcvd)
				reply->good = 0;
	return (0);
}
