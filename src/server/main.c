/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <process.h>
#include "../lib/gen/getopt.h"
#include "service.h"
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
#include "prototypes.h"

static void nullify_objects(void);
static void init_files(void);

#if defined(_WIN32)
static void loc_NTInit(void);
#endif

static int mainpid = 0;

/* Debugging?  If yes call abort() on internal error.  */
int debug = 0;
/* Run as daemon?  If yes, detach from controlling terminal etc. */
int daemonize = 1;

static void
print_usage(char *program_name)
{
#if defined(_WIN32)
    printf("Usage: %s -i -I service_name -r -R service_name -D datadir -e config_file -d\n", program_name);
    printf("-i install service with the default name %s\n", DEFAULT_SERVICE_NAME);
    printf("-r remove service with the default name %s\n", DEFAULT_SERVICE_NAME);
#else
    printf("Usage: %s -D datadir -e config_file -d -p -s\n", program_name);
    printf("-p print flag\n");
    printf("-s stack check flag (include print flag)\n");
#endif
    printf("-d debug mode\n");
}

int
main(int argc, char **argv)
{
    int flags = 0;
#if defined(_WIN32)
    int install_service_set = 0;
    char *service_name = NULL;
    int remove_service_set = 0;
    int datadir_set = 0;
#endif
    char *config_file = NULL;
    int op;
    s_char tbuf[256];

    mainpid = getpid();

#if defined(_WIN32)
    while ((op = getopt(argc, argv, "D:de:iI:rR:h")) != EOF) {
#else
    while ((op = getopt(argc, argv, "D:de:psh")) != EOF) {
#endif
	switch (op) {
	case 'D':
	    datadir = optarg;
#if defined(_WIN32)
	    datadir_set++;
#endif
	    break;
	case 'd':
	    debug++;
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
	    break;
	case 'R':
	    service_name = optarg;
	    /*
	     * fall out
	     */
	case 'r':
	    remove_service_set++;
	    break;
#else
	case 'p':
	    flags |= EMPTH_PRINT;
	    break;
	case 's':
	    flags |= EMPTH_PRINT | EMPTH_STACKCHECK;
	    break;
#endif
	case 'h':
	default:
	    print_usage(argv[0]);
	    return EXIT_FAILURE;
	}
    }

#if defined(_WIN32)
    if ((debug || datadir_set || config_file != NULL) &&
	remove_service_set) {
	logerror("Can't use -d, -D or -e with either "
	    "-r or -R options when starting the server");
	exit(EXIT_FAILURE);
    }
    if (debug && install_service_set) {
	logerror("Can't use -d with either "
	    "-i or -I options when starting the server");
	exit(EXIT_FAILURE);
    }
    if (install_service_set && remove_service_set) {
	logerror("Can't use both -r or -R and -i or -I options when starting "
	    "the server");
	exit(EXIT_FAILURE);
    }
#endif	/* _WIN32 */


#if defined(_WIN32)
    if (remove_service_set)
        return remove_service(service_name);
#endif	/* _WIN32 */

    if (emp_config(config_file) < 0)
	exit(EXIT_FAILURE);

#if defined(_WIN32)
    if (install_service_set)
	return install_service(argv[0], service_name, datadir_set, config_file);
#endif	/* _WIN32 */

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
		printf("Failed to dispatch service (%d)\n", GetLastError());
		exit(EXIT_FAILURE);
	    }
	}
    }
    daemonize = 0;
#endif	/* _WIN32 */

    init_server(flags);
#ifndef _WIN32
    if (daemonize != 0 && flags == 0)
	disassoc();
#endif
    start_server(flags);

#if defined(__linux__) && defined(_EMPTH_POSIX)
    strcpy(tbuf, argv[0]);
    for (op = 1; op < argc; op++) {
	strcat(tbuf, " ");
	strcat(tbuf, argv[op]);
    }
    sprintf(argv[0], "%s (main pid: %d)", tbuf, getpid());
#endif

    empth_exit();

/* We should never get here.  But, just in case... */
    close_files();

#if defined(_WIN32)
    loc_NTTerm();
#endif
    return EXIT_SUCCESS;
}


void
init_server(int flags)
{
#ifdef POSIXSIGNALS
    struct sigaction act;
#endif /* POSIXSIGNALS */

#if defined(_WIN32)
    loc_NTInit();
#endif
    update_policy_check();

    nullify_objects();

#if !defined(_WIN32)
    /* signal() should not be used with mit pthreads. Anyway if u
       have a posix threads u definitly have posix signals -- Sasha */
#if defined (POSIXSIGNALS) || defined (_EMPTH_POSIX)
#ifdef SA_SIGINFO
    act.sa_flags = SA_SIGINFO;
#endif
    sigemptyset(&act.sa_mask);
    act.sa_handler = shutdwn;
    /* pthreads on Linux use SIGUSR1 (*shrug*) so only catch it if not on
       a Linux box running POSIX threads -- STM */
#if !(defined(__linux__) && defined(_EMPTH_POSIX))
    sigaction(SIGUSR1, &act, NULL);
#endif
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    act.sa_handler = panic;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);
#else
    if (debug == 0 && flags == 0) {
	/* pthreads on Linux use SIGUSR1 (*shrug*) so only catch it if not on
	   a Linux box running POSIX threads -- STM */
#if !(defined(__linux__) && defined(_EMPTH_POSIX))
	signal(SIGUSR1, shutdwn);
#endif
	signal(SIGTERM, shutdwn);
	signal(SIGBUS, panic);
	signal(SIGSEGV, panic);
	signal(SIGILL, panic);
	signal(SIGFPE, panic);
	signal(SIGINT, shutdwn);
    }
    signal(SIGPIPE, SIG_IGN);
#endif /* POSIXSIGNALS */
#endif /* _WIN32 */
    empth_init((char **)&player, flags);
    srand(time(NULL));
    global_init();
    shutdown_init();
    player_init();
    ef_init();
    init_files();
    io_init();
    init_nreport();

    if (opt_MOB_ACCESS) {
	/* This fixes up mobility upon restart */
	mobility_init();
    }

    loginit("server");
    logerror("------------------------------------------------------");
    logerror("Empire server (pid %d) started", (int)getpid());
}

void
start_server(int flags)
{
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

static void
init_files(void)
{
    int failed = 0;
    failed |= !ef_open(EF_NATION, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_SECTOR, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_SHIP, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_PLANE, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_LAND, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_NEWS, O_RDWR, 0);
    failed |= !ef_open(EF_LOAN, O_RDWR, 0);
    failed |= !ef_open(EF_TREATY, O_RDWR, 0);
    failed |= !ef_open(EF_NUKE, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_POWER, O_RDWR, 0);
    failed |= !ef_open(EF_TRADE, O_RDWR, 0);
    failed |= !ef_open(EF_MAP, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_BMAP, O_RDWR, EFF_MEM);
    failed |= !ef_open(EF_COMM, O_RDWR, 0);
    failed |= !ef_open(EF_LOST, O_RDWR, 0);
    if (failed) {
	logerror("Missing files, giving up");
	exit(EXIT_FAILURE);
    }
}

void
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
#ifdef POSIXSIGNALS
    struct sigaction act;

    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_DFL;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
#else
    signal(SIGBUS, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
#endif /* POSIXSIGNALS */
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

#if defined(__linux__) && defined(_EMPTH_POSIX)
/* This is a hack to get around the way pthreads work on Linux.  This
   may be useful on other platforms too where threads are turned into
   processes. */
    if (getpid() != mainpid) {
	empth_t *me;

	me = empth_self();
	if (me && me->name) {
	    if (strlen(me->name) > 5) {
		/* Player threads are cleaned up below, so just have
		   them return.  This should work. */
		if (!strncmp("Player", me->name, 6)) {
		    return;
		}
	    }
	}
	/* Not a player thread - must be server thread, so exit */
	empth_exit();
	return;
    }
#endif

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
	logerror("Server shutting down at Deity's request");
    close_files();
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
    for (i = 0; i < pln_maxno; i++) {
	if (opt_NO_HCMS)
	    plchr[i].pl_hcm = 0;
	if (opt_NO_LCMS)
	    plchr[i].pl_lcm = 0;
	if (opt_NO_OIL)
	    plchr[i].pl_fuel = 0;
    }
    for (i = 0; i < lnd_maxno; i++) {
	if (opt_NO_HCMS)
	    lchr[i].l_hcm = 0;
	if (opt_NO_LCMS)
	    lchr[i].l_lcm = 0;
	/* Fix up the military values */
	lchr[i].l_mil = lchr[i].l_item[I_MILIT];
    }
    for (i = 0; i < shp_maxno; i++) {
	if (opt_NO_HCMS)
	    mchr[i].m_hcm = 0;
	if (opt_NO_LCMS)
	    mchr[i].m_lcm = 0;
	if (opt_NO_OIL) {
	    if (mchr[i].m_flags & M_OIL)
		mchr[i].m_name = 0;
	}
    }
    for (i = 0; i < nuk_maxno; i++) {
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
    for (i = 0; i < prd_maxno; i++) {
	for (j = 0; j < MAXPRCON; j++) {
	    if (opt_NO_HCMS && pchr[i].p_ctype[j] == I_HCM)
		pchr[i].p_camt[j] = 0;
	    if (opt_NO_LCMS && pchr[i].p_ctype[j] == I_LCM)
		pchr[i].p_camt[j] = 0;
	    if (opt_NO_OIL && pchr[i].p_ctype[j] == I_OIL)
		pchr[i].p_camt[j] = 0;
	}
    }
}

#if defined(_WIN32)
static void
loc_NTInit()
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
#endif

#if defined(_WIN32)
void
loc_NTTerm()
{
    WSACleanup();
}
#endif
