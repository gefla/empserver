/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#if defined(aix) || defined(linux)
#include <unistd.h>
#endif /* aix or linux */

#include <signal.h>
#include <errno.h>
#if !defined(_WIN32)
#include <sys/ioctl.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <winsock.h>
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
#include "var.h"
#include "product.h"
#include "optlist.h"
#include "global.h"
#include "server.h"
#include "prototypes.h"

s_char program[] = "server";

void nullify_objects(void);
void init_files(void);
void close_files(void);

#if defined(_WIN32)
static void loc_NTInit(void);
static void loc_NTTerm(void);
#endif

extern void mobility_init(void);
extern void mobility_check(void *);
extern void market_update(void *);

#if !defined(_WIN32)
static int mainpid = 0;
#endif

int
main(int argc, char **argv)
{
    time_t now;
    int hour[2];
    int flags = 0;
    int debug = 0;
    int op;
    char *config_file = NULL;
    extern char *optarg;
    s_char tbuf[256];
#ifdef POSIXSIGNALS
    struct sigaction act;
#endif /* POSIXSIGNALS */

#if !defined(_WIN32)
    mainpid = getpid();

    while ((op = getopt(argc, argv, "D:de:psh")) != EOF) {
	switch (op) {
	case 'D':
	    datadir = optarg;
	    break;
	case 'd':
	    debug++;
	    break;
	case 'e':
	    config_file = optarg;
	    break;
	case 'p':
	    flags |= EMPTH_PRINT;
	    break;
	case 's':
	    flags |= EMPTH_PRINT | EMPTH_STACKCHECK;
	    break;
	case 'h':
	default:
	    printf("Usage: %s -d -p -s\n", argv[0]);
	    return 0;
	}
    }
#endif
    if (config_file == NULL) {
	sprintf(tbuf, "%s/econfig", datadir);
	config_file = tbuf;
    }

    logerror("------------------------------------------------------");
#if !defined(_WIN32)
    logerror("Empire server (pid %d) started", getpid());
#else
    logerror("Empire server started");
#endif /* _WIN32 */

#if defined(_WIN32)
    loc_NTInit();
#endif
    emp_config(config_file);
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
    if (debug == 0 && flags == 0) {
	disassoc();
    }
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
	disassoc();
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
    time(&now);
#if !defined(_WIN32)
    srandom(now);
#else
    srand(now);
#endif /* _WIN32 */
    global_init();
    shutdown_init();
    player_init();
    ef_init();
    init_files();
    io_init();
    gamehours(now, hour);

    if (opt_MOB_ACCESS) {
	/* This fixes up mobility upon restart */
	mobility_init();
    }

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
    return 0;
}

void
init_files(void)
{
    ef_open(EF_NATION, O_RDWR, EFF_MEM);
    ef_open(EF_SECTOR, O_RDWR, EFF_MEM);
    ef_open(EF_SHIP, O_RDWR, EFF_MEM);
    ef_open(EF_PLANE, O_RDWR, EFF_MEM);
    ef_open(EF_LAND, O_RDWR, EFF_MEM);
    ef_open(EF_NEWS, O_RDWR, 0);
    ef_open(EF_LOAN, O_RDWR, 0);
    ef_open(EF_TREATY, O_RDWR, 0);
    ef_open(EF_NUKE, O_RDWR, EFF_MEM);
    ef_open(EF_POWER, O_RDWR, 0);
    ef_open(EF_TRADE, O_RDWR, 0);
    ef_open(EF_MAP, O_RDWR, EFF_MEM);
    ef_open(EF_BMAP, O_RDWR, EFF_MEM);
    ef_open(EF_COMM, O_RDWR, 0);
    ef_open(EF_LOST, O_RDWR, 0);
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
void
panic(int sig)
{
#if !defined(_WIN32)
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
#endif /* _WIN32 */
    logerror("server received fatal signal %d", sig);
    log_last_commands();
    close_files();
    _exit(0);
}

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


void
nullify_objects(void)
{
    int i, j;

    if (opt_BIG_CITY) {
	dchr[SCT_CAPIT].d_flg = bigcity_dchr.d_flg;
	dchr[SCT_CAPIT].d_pkg = bigcity_dchr.d_pkg;
	dchr[SCT_CAPIT].d_build = bigcity_dchr.d_build;
	dchr[SCT_CAPIT].d_lcms = bigcity_dchr.d_lcms;
	dchr[SCT_CAPIT].d_hcms = bigcity_dchr.d_hcms;
	dchr[SCT_CAPIT].d_name = bigcity_dchr.d_name;
    }
    for (i = 0; i < pln_maxno; i++) {
	if (opt_NO_HCMS)
	    plchr[i].pl_hcm = 0;
	if (opt_NO_LCMS)
	    plchr[i].pl_lcm = 0;
	if (opt_NO_OIL)
	    plchr[i].pl_fuel = 0;
	if (opt_PLANENAMES)
	    plchr[i].pl_name = plchr[i].pl_planename;
    }
    for (i = 0; i < lnd_maxno; i++) {
	if (opt_NO_HCMS)
	    lchr[i].l_hcm = 0;
	if (opt_NO_LCMS)
	    lchr[i].l_lcm = 0;
	/* Fix up the military values */
	for (j = 0; j < lchr[i].l_nv; j++) {
	    if (lchr[i].l_vtype[j] == V_MILIT)
		lchr[i].l_mil = lchr[i].l_vamt[j];
	}
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
    for (i = 0; i < sct_maxno; i++) {
	if (opt_NO_HCMS)
	    dchr[i].d_hcms = 0;
	if (opt_NO_LCMS)
	    dchr[i].d_lcms = 0;
    }
    for (i = 0; i < prd_maxno; i++) {
	for (j = 0; j < pchr[i].p_nv; j++) {
	    if (opt_NO_HCMS && pchr[i].p_vtype[j] == V_HCM)
		pchr[i].p_vamt[j] = 0;
	    if (opt_NO_LCMS && pchr[i].p_vtype[j] == V_LCM)
		pchr[i].p_vamt[j] = 0;
	    if (opt_NO_OIL && pchr[i].p_vtype[j] == V_OIL)
		pchr[i].p_vamt[j] = 0;
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
static void
loc_NTTerm()
{
    WSACleanup();
}
#endif
