/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ---
 *
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  main.c: Thread and signal initialization for Empire Server
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996, 1998
 *     Doug Hay, 1998
 */

#include <signal.h>
#if !defined(_WIN32)
#include <sys/ioctl.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32
#include <winsock2.h>
#undef NS_ALL
#include <process.h>
#include "../lib/gen/getopt.h"
#include "service.h"
#include "direct.h"
#endif

#include "misc.h"
#include "nat.h"
#include "file.h"
#include "player.h"
#include "empthread.h"
#include "plane.h"
#include "nuke.h"
#include "land.h"
#include "ship.h"
#include "sect.h"
#include "product.h"
#include "optlist.h"
#include "server.h"
#include "version.h"
#include "prototypes.h"

static void nullify_objects(void);
static void init_files(void);
static void close_files(void);
static void create_pidfile(char *, pid_t);

#if defined(_WIN32)
static void loc_NTInit(void);
static void loc_NTTerm(void);
#endif

static char pidfname[] = "server.pid";

/* Run as daemon?  If yes, detach from controlling terminal etc. */
int daemonize = 1;

static void
print_usage(char *program_name)
{
    printf("Usage: %s [OPTION]...\n"
	   "  -d              debug mode\n"
	   "  -e CONFIG-FILE  configuration file\n"
	   "                  (default %s)\n"
	   "  -h              display this help and exit\n"
#ifdef _WIN32
	   "  -i              install service `%s'\n"
	   "  -I NAME         install service NAME\n"
#endif
	   "  -p              threading debug mode, implies -d\n"
#ifdef _WIN32
	   "  -r              remove service `%s'\n"
	   "  -R NAME         remove service NAME\n"
#endif
	   "  -s              enable stack checking\n"
	   "  -v              display version information and exit\n",
	   program_name, dflt_econfig
#ifdef _WIN32
	   , DEFAULT_SERVICE_NAME, DEFAULT_SERVICE_NAME
#endif
	);
}

int
main(int argc, char **argv)
{
    int flags = 0;
#if defined(_WIN32)
    int install_service_set = 0;
    char *program_name = NULL;
    char *service_name = NULL;
    int remove_service_set = 0;
#endif
    char *config_file = NULL;
    int op;

#ifdef _WIN32
# define XOPTS "iI:rR:"
#else
# define XOPTS
#endif
    while ((op = getopt(argc, argv, "de:hpsv" XOPTS)) != EOF) {
	switch (op) {
	case 'p':
	    flags |= EMPTH_PRINT;
	    /* fall through */
	case 'd':
	    debug = 1;
	    daemonize = 0;
	    break;
	case 'e':
	    config_file = optarg;
	    break;
#if defined(_WIN32)
	case 'I':
	    service_name = optarg;
	    /*
	     * fall out
	     */
	case 'i':
	    install_service_set++;
	    break;
	case 'R':
	    service_name = optarg;
	    /*
	     * fall out
	     */
	case 'r':
	    remove_service_set++;
	    break;
#endif
	case 's':
	    flags |= EMPTH_STACKCHECK;
	    break;
	case 'v':
	    printf("Wolfpack Empire %d.%d.%d\n",
		   EMP_VERS_MAJOR, EMP_VERS_MINOR, EMP_VERS_PATCH);
	    return EXIT_SUCCESS;
	case 'h':
	    print_usage(argv[0]);
	    return EXIT_SUCCESS;
	default:
	    fprintf(stderr, "Try -h for help.\n");
	    return EXIT_FAILURE;
	}
    }

#if defined(_WIN32)
    if ((debug || flags || config_file != NULL) &&
	remove_service_set) {
	fprintf(stderr, "Can't use -p, -s, -d or -e with either "
	    "-r or -R options\n");
	exit(EXIT_FAILURE);
    }
    if ((debug || flags) && install_service_set) {
	fprintf(stderr, "Can't use -d, -p or -s with either "
	    "-i or -I options\n");
	exit(EXIT_FAILURE);
    }
    if (install_service_set && remove_service_set) {
	fprintf(stderr, "Can't use both -r or -R and -i or -I "
	    "options\n");
	exit(EXIT_FAILURE);
    }
#endif	/* _WIN32 */


#if defined(_WIN32)
    if (remove_service_set)
        return remove_service(service_name);
    if (install_service_set) {
	program_name = _fullpath(NULL, argv[0], 0);
	if (config_file != NULL)
	    config_file = _fullpath(NULL, config_file, 0);
    }
#endif	/* _WIN32 */

    if (emp_config(config_file) < 0)
	exit(EXIT_FAILURE);
    if (chdir(datadir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n", datadir, strerror(errno));
	exit(EXIT_FAILURE);
    }

#if defined(_WIN32)
    if (install_service_set)
	return install_service(program_name, service_name, config_file);
#endif	/* _WIN32 */

    init_server();

#if defined(_WIN32)
    if (daemonize != 0) {
	SERVICE_TABLE_ENTRY DispatchTable[]={{"Empire Server", service_main},{NULL, NULL}};
	if (StartServiceCtrlDispatcher(DispatchTable))
	    return 0;
	else {
	    /*
	     * If it is service startup error then exit otherwise
	     * start server in the foreground
	     */
	    if (GetLastError() != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
		logerror("Failed to dispatch service (%d)", GetLastError());
		finish_server();
		exit(EXIT_FAILURE);
	    }
	}
    }
    daemonize = 0;
#else  /* !_WIN32 */
    if (daemonize) {
	if (disassoc() < 0) {
	    logerror("Can't become daemon (%s)", strerror(errno));
	    _exit(1);
	}
    }
#endif /* !_WIN32 */
    start_server(flags);

    empth_exit();

    CANT_HAPPEN("main thread terminated");
    finish_server();
    return EXIT_SUCCESS;
}


/*
 * Initialize for serving, acquire resources.
 */
void
init_server(void)
{
    srandom(time(NULL));
#if defined(_WIN32)
    loc_NTInit();
#endif
    update_policy_check();
    nullify_objects();
    global_init();
    shutdown_init();
    player_init();
    if (ef_load() < 0)
	exit(EXIT_FAILURE);
    ef_init_srv();
    init_files();
    io_init();
    init_nreport();

    if (opt_MOB_ACCESS) {
	/* This fixes up mobility upon restart */
	mobility_init();
    }

    loginit("server");
}

/*
 * Start serving.
 */
void
start_server(int flags)
{
    pid_t pid;
#if !defined(_WIN32)
    struct sigaction act;
#endif

    pid = getpid();
    create_pidfile(pidfname, pid);
    logerror("------------------------------------------------------");
    logerror("Empire server (pid %d) started", (int)pid);

#if !defined(_WIN32)
    /* signal() should not be used with mit pthreads. Anyway if u
       have a posix threads u definitly have posix signals -- Sasha */
    sigemptyset(&act.sa_mask);
    act.sa_handler = shutdwn;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    act.sa_handler = panic;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);
#endif /* !_WIN32 */

    empth_init((void **)&player, flags);

    empth_create(PP_ACCEPT, player_accept, (50 * 1024), flags,
		 "AcceptPlayers", "Accept network connections", 0);
    empth_create(PP_KILLIDLE, player_kill_idle, (50 * 1024), flags,
		 "KillIdle", "Kills idle player connections", 0);
    empth_create(PP_SCHED, update_sched, (50 * 1024), flags, "UpdateSched",
		 "Schedules updates to occur", 0);
    empth_create(PP_TIMESTAMP, delete_lostitems, (50 * 1024), flags,
		 "DeleteItems", "Deletes old lost items", 0);
    if (opt_MOB_ACCESS) {
	/* Start the mobility access check thread */
	empth_create(PP_TIMESTAMP, mobility_check, (50 * 1024), flags,
		     "MobilityCheck", "Writes the timestamp file", 0);
    }

    if (opt_MARKET) {
	empth_create(PP_TIMESTAMP, market_update, (50 * 1024), flags,
		     "MarketUpdate", "Updates the market", 0);
    }
}

/*
 * Finish serving, release resources.
 */
void
finish_server(void)
{
    close_files();
#if defined(_WIN32)
    loc_NTTerm();
#endif
    remove(pidfname);
}

static void
create_pidfile(char *fname, pid_t pid)
{
    FILE *pidf = fopen(fname, "w");
    if (!pidf
	|| fprintf(pidf, "%d\n", (int)pid) < 0
	|| fclose(pidf)) {
	logerror("Can't write PID file (%s)", strerror(errno));
	exit(1);
    }
}

static void
init_files(void)
{
    int failed = 0;
    failed |= !ef_open(EF_NATION, EFF_MEM);
    failed |= !ef_open(EF_SECTOR, EFF_MEM);
    failed |= !ef_open(EF_SHIP, EFF_MEM);
    failed |= !ef_open(EF_PLANE, EFF_MEM);
    failed |= !ef_open(EF_LAND, EFF_MEM);
    failed |= !ef_open(EF_NEWS, 0);
    failed |= !ef_open(EF_LOAN, 0);
    failed |= !ef_open(EF_TREATY, 0);
    failed |= !ef_open(EF_NUKE, EFF_MEM);
    failed |= !ef_open(EF_POWER, 0);
    failed |= !ef_open(EF_TRADE, 0);
    failed |= !ef_open(EF_MAP, EFF_MEM);
    failed |= !ef_open(EF_BMAP, EFF_MEM);
    failed |= !ef_open(EF_COMM, 0);
    failed |= !ef_open(EF_LOST, 0);
    if (failed) {
	logerror("Missing files, giving up");
	exit(EXIT_FAILURE);
    }
}

static void
close_files(void)
{
    ef_close(EF_NATION);
    ef_close(EF_SECTOR);
    ef_close(EF_SHIP);
    ef_close(EF_PLANE);
    ef_close(EF_LAND);
    ef_close(EF_NEWS);
    ef_close(EF_LOAN);
    ef_close(EF_TREATY);
    ef_close(EF_NUKE);
    ef_close(EF_POWER);
    ef_close(EF_TRADE);
    ef_close(EF_MAP);
    ef_close(EF_COMM);
    ef_close(EF_BMAP);
    ef_close(EF_LOST);
}

/* we're going down.  try to close the files at least */
#if !defined(_WIN32)
void
panic(int sig)
{
    struct sigaction act;

    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_DFL;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    logerror("server received fatal signal %d", sig);
    log_last_commands();
    close_files();
    if (CANT_HAPPEN(sig != SIGBUS && sig != SIGSEGV
		    && sig != SIGILL && sig != SIGFPE))
	_exit(1);
    if (raise(sig))
	_exit(1);
}
#endif /* _WIN32 */

void
shutdwn(int sig)
{
    struct player *p;
    time_t now;

    logerror("Shutdown commencing (cleaning up threads.)");

    for (p = player_next(0); p != 0; p = player_next(p)) {
	if (p->state != PS_PLAYING)
	    continue;
	pr_flash(p, "Server shutting down...\n");
	p->state = PS_SHUTDOWN;
	p->aborted++;
	if (p->command) {
	    pr_flash(p, "Shutdown aborting command\n");
	}
	empth_wakeup(p->proc);
    }

    if (!sig) {
	/* Sleep and let some cleanup happen - note this doesn't work
	   when called from a signal handler, since we may or may not
	   be in the right thread.  So we just pass by and kill 'em
	   all. */
	time(&now);
	empth_sleep(now + 1);
    }

    for (p = player_next(0); p != 0; p = player_next(p)) {
	p->state = PS_KILL;
	p->aborted++;
	empth_terminate(p->proc);
	p = player_delete(p);
    }
    if (sig)
	logerror("Server shutting down on signal %d", sig);
    else
	logerror("Server shutting down at deity's request");
    finish_server();

#if defined(_WIN32)
    if (daemonize) {
        stop_service();
	return;
    }
#endif
    _exit(0);
}


static void
nullify_objects(void)
{
    int i, j;

    if (opt_BIG_CITY)
	dchr[SCT_CAPIT] = bigcity_dchr;
    if (opt_NO_LCMS)
	dchr[SCT_LIGHT].d_cost = -1;
    if (opt_NO_HCMS)
	dchr[SCT_HEAVY].d_cost = -1;
    if (opt_NO_OIL) {
	dchr[SCT_OIL].d_cost = -1;
	dchr[SCT_REFINE].d_cost = -1;
    }
    for (i = 0; plchr[i].pl_name; i++) {
	if (opt_NO_HCMS)
	    plchr[i].pl_hcm = 0;
	if (opt_NO_LCMS)
	    plchr[i].pl_lcm = 0;
	if (opt_NO_OIL)
	    plchr[i].pl_fuel = 0;
    }
    for (i = 0; lchr[i].l_name; i++) {
	if (opt_NO_HCMS)
	    lchr[i].l_hcm = 0;
	if (opt_NO_LCMS)
	    lchr[i].l_lcm = 0;
	/* Fix up the military values */
	lchr[i].l_mil = lchr[i].l_item[I_MILIT];
    }
    for (i = 0; mchr[i].m_name; i++) {
	if (opt_NO_HCMS)
	    mchr[i].m_hcm = 0;
	if (opt_NO_LCMS)
	    mchr[i].m_lcm = 0;
	if (opt_NO_OIL)
	    mchr[i].m_flags &= ~M_OIL;
    }
    for (i = 0; nchr[i].n_name; i++) {
	if (opt_NO_HCMS)
	    nchr[i].n_hcm = 0;
	if (opt_NO_LCMS)
	    nchr[i].n_lcm = 0;
    }
    for (i = 0; i <= SCT_MAXDEF; i++) {
	if (opt_NO_HCMS)
	    dchr[i].d_hcms = 0;
	if (opt_NO_LCMS)
	    dchr[i].d_lcms = 0;
    }
    for (i = 0; pchr[i].p_name; i++) {
	for (j = 0; j < MAXPRCON; j++) {
	    if (opt_NO_HCMS && pchr[i].p_ctype[j] == I_HCM)
		pchr[i].p_camt[j] = 0;
	    if (opt_NO_LCMS && pchr[i].p_ctype[j] == I_LCM)
		pchr[i].p_camt[j] = 0;
	    if (opt_NO_OIL && pchr[i].p_ctype[j] == I_OIL)
		pchr[i].p_camt[j] = 0;
	}
    }
    for (i = 0; intrchr[i].in_name; i++) {
	if (opt_NO_HCMS)
	    intrchr[i].in_hcms = 0;
	if (opt_NO_LCMS)
	    intrchr[i].in_lcms = 0;
    }
}

#if defined(_WIN32)
static void
loc_NTInit(void)
{
    int rc;
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 0);
    rc = WSAStartup(wVersionRequested, &wsaData);
    if (rc != 0) {
	logerror("WSAStartup failed.  %d", rc);
	_exit(1);
    }
}

static void
loc_NTTerm(void)
{
    WSACleanup();
}
#endif
