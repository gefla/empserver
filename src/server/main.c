/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  main.c: Empire Server main, startup and shutdown
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1996, 1998
 *     Doug Hay, 1998
 *     Ron Koenderink, 2004-2005
 *     Markus Armbruster, 2005-2006
 */

#include <config.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>

#if defined(_WIN32)
#include <winsock2.h>
#undef NS_ALL
#include <process.h>
#include <direct.h>
#include "service.h"
#include "../lib/gen/getopt.h"
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "empio.h"
#include "empthread.h"
#include "file.h"
#include "journal.h"
#include "land.h"
#include "misc.h"
#include "nat.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "product.h"
#include "prototypes.h"
#include "sect.h"
#include "server.h"
#include "ship.h"
#include "version.h"

static void create_pidfile(char *, pid_t);

#if defined(_WIN32)
static void loc_NTInit(void);
static void loc_NTTerm(void);
#endif

static char pidfname[] = "server.pid";

/* Run as daemon?  If yes, detach from controlling terminal etc. */
static int daemonize = 1;

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
    int op, sig;

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
	    /* fall through */
	case 'i':
	    install_service_set++;
	    break;
	case 'R':
	    service_name = optarg;
	    /* fall through */
	case 'r':
	    remove_service_set++;
	    break;
#endif	/* _WIN32 */
	case 's':
	    flags |= EMPTH_STACKCHECK;
	    break;
	case 'v':
	    printf("%s\n\n%s", version, legal);
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
    ef_init();
    if (chdir(configdir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n",
		configdir, strerror(errno));
	exit(EXIT_FAILURE);
    }
    if (chdir(builtindir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n",
		builtindir, strerror(errno));
	exit(EXIT_FAILURE);
    }
    if (read_builtin_tables() < 0)
	exit(EXIT_FAILURE);
    if (chdir(configdir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n",
		configdir, strerror(errno));
	exit(EXIT_FAILURE);
    }
    if (read_custom_tables() < 0)
	exit(EXIT_FAILURE);
    if (chdir(gamedir)) {
	fprintf(stderr, "Can't chdir to %s (%s)\n",
		gamedir, strerror(errno));
	exit(EXIT_FAILURE);
    }

#if defined(_WIN32)
    if (install_service_set)
	return install_service(program_name, service_name, config_file);
#endif	/* _WIN32 */

    init_server();

#if defined(_WIN32)
    if (daemonize != 0) {
	SERVICE_TABLE_ENTRY DispatchTable[]={
	    {"Empire Server", service_main},
	    {NULL, NULL}
	};
	if (StartServiceCtrlDispatcher(DispatchTable))
	    return 0;
	else {
	    /*
	     * If it is service startup error then exit otherwise
	     * start server in the foreground
	     */
	    if (GetLastError() != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
		logerror("Failed to dispatch service (%lu)",
			 GetLastError());
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
	    exit(1);
	}
    }
#endif /* !_WIN32 */
    start_server(flags);

    for (;;) {
	sig = empth_wait_for_signal();
#ifdef SIGHUP
	if (sig == SIGHUP) {
	    journal_reopen();
	    update_reschedule();
	    continue;
	}
#endif
	break;
    }

    shutdwn(sig);
    CANT_REACH();
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
    player_init();
    ef_init_srv();
    io_init();
    init_nreport();

    loginit("server");

    if (opt_MOB_ACCESS) {
	/* This fixes up mobility upon restart */
	mobility_init();
    }
}

/*
 * Start serving.
 */
void
start_server(int flags)
{
    pid_t pid;

    pid = getpid();
    create_pidfile(pidfname, pid);
    logerror("------------------------------------------------------");
    logerror("Empire server (pid %d) started", (int)pid);

    empth_init((void **)&player, flags);

    if (journal_startup() < 0)
	exit(1);

    empth_create(PP_ACCEPT, player_accept, (50 * 1024), flags,
		 "AcceptPlayers", 0);
    empth_create(PP_KILLIDLE, player_kill_idle, (50 * 1024), flags,
		 "KillIdle", 0);
    empth_create(PP_TIMESTAMP, delete_lostitems, (50 * 1024), flags,
		 "DeleteItems", 0);
    if (opt_MOB_ACCESS) {
	empth_create(PP_TIMESTAMP, mobility_check, (50 * 1024), flags,
		     "MobilityCheck", 0);
    }

    market_init();
    update_init();
}

/*
 * Finish serving, release resources.
 */
void
finish_server(void)
{
    ef_fin_srv();
#if defined(_WIN32)
    loc_NTTerm();
#endif
    journal_shutdown();
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

void
shutdwn(int sig)
{
    struct player *p;

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
    empth_rwlock_wrlock(update_lock);
    /* rely on player_kill_idle() for killing hung player threads */
    if (sig)
	logerror("Server shutting down on signal %d", sig);
    else
	logerror("Server shutting down at deity's request");
    finish_server();

#if defined(_WIN32)
    if (daemonize)
        stop_service();
#endif
    exit(0);
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
	exit(1);
    }
}

static void
loc_NTTerm(void)
{
    WSACleanup();
}
#endif
