
/*
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE".  Use of the mrouted program represents acceptance of
 * the terms and conditions listed in that file.
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 */

/*
 * Written by Steve Deering, Stanford University, February 1989.
 *
 * (An earlier version of DVMRP was implemented by David Waitzman of
 *  BBN STC by extending Berkeley's routed program.  Some of Waitzman's
 *  extensions have been incorporated into mrouted, but none of the
 *  original routed code has been adopted.)
 */


#include "defs.h"
#include <stdarg.h>
#include <fcntl.h>
#include <util.h>

#ifndef lint
static char rcsid[] = "@(#) $ABSD$";
#endif

extern char *configfilename;
char versionstring[100];

static char dumpfilename[] = _PATH_MROUTED_DUMP;
static char cachefilename[] = _PATH_MROUTED_CACHE;
static char genidfilename[] = _PATH_MROUTED_GENID;

int cache_lifetime	= DEFAULT_CACHE_LIFETIME;
int max_prune_lifetime	= DEFAULT_CACHE_LIFETIME * 2;

int debug = 0;
u_char pruning = 1;	/* Enable pruning by default */

#define NHANDLERS	2

static struct ihandler {
    int fd;			/* File descriptor		 */
    ihfunc_t func;		/* Function to call with &fd_set */
} ihandlers[NHANDLERS];
static int nhandlers = 0;

/*
 * Forward declarations.
 */
static void fasttimer(int);
static void done(int);
static void dump(int);
static void fdump(int);
static void cdump(int);
static void restart(int);
static void timer(void);
static void cleanup(void);
static void resetlogging(void *);

int
register_input_handler(int fd, ihfunc_t func)
{
    if (nhandlers >= NHANDLERS)
	return -1;

    ihandlers[nhandlers].fd = fd;
    ihandlers[nhandlers++].func = func;

    return 0;
}

int
main(int argc, char *argv[])
{
    register int recvlen;
    int dummy;
    FILE *fp;
    struct timeval tv;
    u_int32_t prev_genid;
    int vers;
    fd_set rfds, readers;
    int nfds, n, i, ch;
    sigset_t mask, omask;
    const char *errstr;

    if (geteuid() != 0) {
	fprintf(stderr, "must be root\n");
	exit(1);
    }
    setlinebuf(stderr);

    while ((ch = getopt(argc, argv, "c:d::p")) != -1) {
	    switch (ch) {
	    case 'c':
		    configfilename = optarg;
		    break;
	    case 'd':
		    if (!optarg)
			    debug = DEFAULT_DEBUG;
		    else {
			    debug = strtonum(optarg, 0, 3, &errstr);
			    if (errstr) {
				    warnx("debug level %s", errstr);
				    debug = DEFAULT_DEBUG;
			    }
		    }
		    break;
	    case 'p':
		    pruning = 0;
		    break;
	    default:
		    goto usage;
	    }
    }
    argc -= optind;
    argv += optind;
    
    if (argc > 0) {
usage:	fprintf(stderr,
		"usage: mrouted [-p] [-c config_file] [-d [debug_level]]\n");
	exit(1);
    }

    if (debug == 0) {
	/*
	 * Detach from the terminal
	 */
	int t;

	if (fork()) exit(0);
	(void)close(0);
	(void)close(1);
	(void)close(2);
	(void)open("/", 0);
	(void)dup2(0, 1);
	(void)dup2(0, 2);
#ifdef SYSV
	(void)setpgrp();
#else
#ifdef TIOCNOTTY
	t = open("/dev/tty", 2);
	if (t >= 0) {
	    (void)ioctl(t, TIOCNOTTY, (char *)0);
	    (void)close(t);
	}
#else
	if (setsid() < 0)
	    perror("setsid");
#endif
#endif
    }
    else
	fprintf(stderr, "debug level %u\n", debug);

#ifdef LOG_DAEMON
    (void)openlog("mrouted", LOG_PID, LOG_DAEMON);
    (void)setlogmask(LOG_UPTO(LOG_NOTICE));
#else
    (void)openlog("mrouted", LOG_PID);
#endif
    snprintf(versionstring, sizeof versionstring, "mrouted version %d.%d",
			PROTOCOL_VERSION, MROUTED_VERSION);

    logit(LOG_NOTICE, 0, "%s", versionstring);

#ifdef SYSV
    srand48(time(NULL));
#else
    srandom(gethostid());
#endif

    /*
     * Get generation id
     */
    gettimeofday(&tv, 0);
    dvmrp_genid = tv.tv_sec;

    fp = fopen(genidfilename, "r");
    if (fp != NULL) {
	fscanf(fp, "%d", &prev_genid);
	if (prev_genid == dvmrp_genid)
	    dvmrp_genid++;
	(void) fclose(fp);
    }

    fp = fopen(genidfilename, "w");
    if (fp != NULL) {
	fprintf(fp, "%d", dvmrp_genid);
	(void) fclose(fp);
    }

    callout_init();
    init_igmp();
    init_routes();
    init_ktable();
    k_init_dvmrp();		/* enable DVMRP routing in kernel */

#ifndef OLD_KERNEL
    vers = k_get_version();
    /*XXX
     * This function must change whenever the kernel version changes
     */
    if ((((vers >> 8) & 0xff) != 3) ||
	 ((vers & 0xff) != 5))
	logit(LOG_ERR, 0, "kernel (v%d.%d)/mrouted (v%d.%d) version mismatch",
		(vers >> 8) & 0xff, vers & 0xff,
		PROTOCOL_VERSION, MROUTED_VERSION);
#endif

    init_vifs();

#ifdef RSRR
    rsrr_init();
#endif /* RSRR */

    /*
     * Allow cleanup if unexpected exit.  Apparently some architectures
     * have a kernel bug where closing the socket doesn't do an
     * ip_mrouter_done(), so we attempt to do it on exit.
     */
    atexit(cleanup);

    if (debug)
	fprintf(stderr, "pruning %s\n", pruning ? "on" : "off");

    pidfile(NULL);

    (void)signal(SIGALRM, fasttimer);

    (void)signal(SIGHUP,  restart);
    (void)signal(SIGTERM, done);
    (void)signal(SIGINT,  done);
    (void)signal(SIGUSR1, fdump);
    (void)signal(SIGUSR2, cdump);
    if (debug != 0)
	(void)signal(SIGQUIT, dump);

    FD_ZERO(&readers);
    if (igmp_socket >= FD_SETSIZE)
	logit(LOG_ERR, 0, "descriptor too big");
    FD_SET(igmp_socket, &readers);
    nfds = igmp_socket + 1;
    for (i = 0; i < nhandlers; i++) {
	if (ihandlers[i].fd >= FD_SETSIZE)
	    logit(LOG_ERR, 0, "descriptor too big");
	FD_SET(ihandlers[i].fd, &readers);
	if (ihandlers[i].fd >= nfds)
	    nfds = ihandlers[i].fd + 1;
    }

    /*
     * Install the vifs in the kernel as late as possible in the
     * initialization sequence.
     */
    init_installvifs();

    if (debug >= 2) dump(0);

    /* Start up the log rate-limiter */
    resetlogging(NULL);

    (void)alarm(1);	 /* schedule first timer interrupt */

    /*
     * Main receive loop.
     */
    dummy = 0;
    for(;;) {
	bcopy((char *)&readers, (char *)&rfds, sizeof(rfds));
	if ((n = select(nfds, &rfds, NULL, NULL, NULL)) < 0) {
            if (errno != EINTR) /* SIGALRM is expected */
                logit(LOG_WARNING, errno, "select failed");
            continue;
        }

	if (FD_ISSET(igmp_socket, &rfds)) {
	    recvlen = recvfrom(igmp_socket, recv_buf, RECV_BUF_SIZE,
			       0, NULL, &dummy);
	    if (recvlen < 0) {
		if (errno != EINTR) logit(LOG_ERR, errno, "recvfrom");
		continue;
	    }
	    (void)sigemptyset(&mask);
	    (void)sigaddset(&mask, SIGALRM);
	    if (sigprocmask(SIG_BLOCK, &mask, &omask) < 0)
		    logit(LOG_ERR, errno, "sigprocmask");
	    accept_igmp(recvlen);
	    (void)sigprocmask(SIG_SETMASK, &omask, NULL);
        }

	for (i = 0; i < nhandlers; i++) {
	    if (FD_ISSET(ihandlers[i].fd, &rfds)) {
		(*ihandlers[i].func)(ihandlers[i].fd, &rfds);
	    }
	}
    }
}


/*
 * routine invoked every second.  Its main goal is to cycle through
 * the routing table and send partial updates to all neighbors at a
 * rate that will cause the entire table to be sent in ROUTE_REPORT_INTERVAL
 * seconds.  Also, every TIMER_INTERVAL seconds it calls timer() to
 * do all the other time-based processing.
 */
static void
fasttimer(int i)
{
    static unsigned int tlast;
    static unsigned int nsent;
    register unsigned int t = tlast + 1;
    register int n;

    /*
     * if we're in the last second, send everything that's left.
     * otherwise send at least the fraction we should have sent by now.
     */
    if (t >= ROUTE_REPORT_INTERVAL) {
	register int nleft = nroutes - nsent;
	while (nleft > 0) {
	    if ((n = report_next_chunk()) <= 0)
		break;
	    nleft -= n;
	}
	tlast = 0;
	nsent = 0;
    } else {
	register unsigned int ncum = nroutes * t / ROUTE_REPORT_INTERVAL;
	while (nsent < ncum) {
	    if ((n = report_next_chunk()) <= 0)
		break;
	    nsent += n;
	}
	tlast = t;
    }
    if ((t % TIMER_INTERVAL) == 0)
	timer();

    age_callout_queue();/* Advance the timer for the callout queue
				for groups */
    alarm(1);
}

/*
 * The 'virtual_time' variable is initialized to a value that will cause the
 * first invocation of timer() to send a probe or route report to all vifs
 * and send group membership queries to all subnets for which this router is
 * querier.  This first invocation occurs approximately TIMER_INTERVAL seconds
 * after the router starts up.   Note that probes for neighbors and queries
 * for group memberships are also sent at start-up time, as part of initial-
 * ization.  This repetition after a short interval is desirable for quickly
 * building up topology and membership information in the presence of possible
 * packet loss.
 *
 * 'virtual_time' advances at a rate that is only a crude approximation of
 * real time, because it does not take into account any time spent processing,
 * and because the timer intervals are sometimes shrunk by a random amount to
 * avoid unwanted synchronization with other routers.
 */

static u_long virtual_time = 0;


/*
 * Timer routine.  Performs periodic neighbor probing, route reporting, and
 * group querying duties, and drives various timers in routing entries and
 * virtual interface data structures.
 */
static void
timer(void)
{
    age_routes();	/* Advance the timers in the route entries     */
    age_vifs();		/* Advance the timers for neighbors */
    age_table_entry();	/* Advance the timers for the cache entries */

    if (virtual_time % GROUP_QUERY_INTERVAL == 0) {
	/*
	 * Time to query the local group memberships on all subnets
	 * for which this router is the elected querier.
	 */
	query_groups();
    }

    if (virtual_time % NEIGHBOR_PROBE_INTERVAL == 0) {
	/*
	 * Time to send a probe on all vifs from which no neighbors have
	 * been heard.  Also, check if any inoperative interfaces have now
	 * come up.  (If they have, they will also be probed as part of
	 * their initialization.)
	 */
	probe_for_neighbors();

	if (vifs_down)
	    check_vif_state();
    }

    delay_change_reports = FALSE;
    if (routes_changed) {
	/*
	 * Some routes have changed since the last timer interrupt, but
	 * have not been reported yet.  Report the changed routes to all
	 * neighbors.
	 */
	report_to_all_neighbors(CHANGED_ROUTES);
    }

    /*
     * Advance virtual time
     */
    virtual_time += TIMER_INTERVAL;
}


/*
 * On termination, let everyone know we're going away.
 */
static void
done(int i)
{
    logit(LOG_NOTICE, 0, "%s exiting", versionstring);
    cleanup();
    _exit(1);
}

static void
cleanup(void)
{
    static in_cleanup = 0;

    if (!in_cleanup) {
	in_cleanup++;
#ifdef RSRR
	rsrr_clean();
#endif /* RSRR */
	expire_all_routes();
	report_to_all_neighbors(ALL_ROUTES);
	k_stop_dvmrp();
    }
}


/*
 * Dump internal data structures to stderr.
 */
static void
dump(int i)
{
    dump_vifs(stderr);
    dump_routes(stderr);
}


/*
 * Dump internal data structures to a file.
 */
static void
fdump(int i)
{
    FILE *fp;

    fp = fopen(dumpfilename, "w");
    if (fp != NULL) {
	dump_vifs(fp);
	dump_routes(fp);
	(void) fclose(fp);
    }
}


/*
 * Dump local cache contents to a file.
 */
static void
cdump(int i)
{
    FILE *fp;

    fp = fopen(cachefilename, "w");
    if (fp != NULL) {
	dump_cache(fp);
	(void) fclose(fp);
    }
}


/*
 * Restart mrouted
 */
static void
restart(int i)
{
    sigset_t mask, omask;

    logit(LOG_NOTICE, 0, "%s restart", versionstring);

    /*
     * reset all the entries
     */
    (void)sigemptyset(&mask);
    (void)sigaddset(&mask, SIGALRM);
    if (sigprocmask(SIG_BLOCK, &mask, &omask) < 0)
	logit(LOG_ERR, errno, "sigprocmask");
    free_all_prunes();
    free_all_routes();
    stop_all_vifs();
    k_stop_dvmrp();
    close(igmp_socket);
    close(udp_socket);

    /*
     * start processing again
     */
    dvmrp_genid++;
    pruning = 1;

    init_igmp();
    init_routes();
    init_ktable();
    init_vifs();
    k_init_dvmrp();		/* enable DVMRP routing in kernel */
    init_installvifs();

    (void)sigprocmask(SIG_SETMASK, &omask, NULL);
}

#define LOG_MAX_MSGS	20	/* if > 20/minute then shut up for a while */
#define LOG_SHUT_UP	600	/* shut up for 10 minutes */
static int log_nmsgs = 0;

static void
resetlogging(void *arg)
{
    int nxttime = 60;
    void *narg = NULL;

    if (arg == NULL && log_nmsgs > LOG_MAX_MSGS) {
	nxttime = LOG_SHUT_UP;
	narg = (void *)&log_nmsgs;	/* just need some valid void * */
	syslog(LOG_WARNING, "logging too fast, shutting up for %d minutes",
			LOG_SHUT_UP / 60);
    } else {
	log_nmsgs = 0;
    }

    timer_setTimer(nxttime, resetlogging, narg);
}

/*
 * Log errors and other messages to the system log daemon and to stderr,
 * according to the severity of the message and the current debug level.
 * For errors of severity LOG_ERR or worse, terminate the program.
 */
void
logit(int severity, int syserr, char *format, ...)
{
    va_list ap;
    static char fmt[211] = "warning - ";
    char *msg;
    char tbuf[20];
    struct timeval now;
    struct tm *thyme;
    time_t t;

    va_start(ap, format);
    vsnprintf(&fmt[10], sizeof fmt - 10, format, ap);
    va_end(ap);
    msg = (severity == LOG_WARNING) ? fmt : &fmt[10];

    switch (debug) {
	case 0: break;
	case 1: if (severity > LOG_NOTICE) break;
	case 2: if (severity > LOG_INFO  ) break;
	default:
	    gettimeofday(&now,NULL);
	    t = now.tv_sec;
	    thyme = localtime(&t);
	    strftime(tbuf, sizeof(tbuf), "%X.%%03d ", thyme);
	    fprintf(stderr, tbuf, now.tv_usec / 1000);
	    fprintf(stderr, "%s", msg);
	    if (syserr == 0)
		fprintf(stderr, "\n");
	    else if (syserr < sys_nerr)
		fprintf(stderr, ": %s\n", sys_errlist[syserr]);
	    else
		fprintf(stderr, ": errno %d\n", syserr);
    }

    if (severity <= LOG_NOTICE) {
	if (log_nmsgs++ < LOG_MAX_MSGS) {
	    if (syserr != 0) {
		errno = syserr;
		syslog(severity, "%s: %m", msg);
	    } else
		syslog(severity, "%s", msg);
	}

	if (severity <= LOG_ERR) exit(1);
    }
}

#ifdef DEBUG_MFC
void
md_logit(int what, u_int32_t origin, u_int32_t mcastgrp)
{
    static FILE *f = NULL;
    struct timeval tv;
    u_int32_t buf[4];

    if (!f) {
	if ((f = fopen("/tmp/mrouted.clog", "w")) == NULL) {
	    logit(LOG_ERR, errno, "open /tmp/mrouted.clog");
	}
    }

    gettimeofday(&tv, NULL);
    buf[0] = tv.tv_sec;
    buf[1] = what;
    buf[2] = origin;
    buf[3] = mcastgrp;

    fwrite(buf, sizeof(u_int32_t), 4, f);
}
#endif
