/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 *     Ron Koenderink, 2004-2009
 *     Markus Armbruster, 2005-2012
 */

#include <config.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include <unistd.h>

#if defined(_WIN32)
#include <process.h>
#include "service.h"
#include "sys/socket.h"
#endif

#include "empio.h"
#include "empthread.h"
#include "file.h"
#include "journal.h"
#include "match.h"
#include "misc.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "product.h"
#include "prototypes.h"
#include "sect.h"
#include "server.h"
#include "version.h"

static void ignore(void);
static void crash_dump(void);
static void init_server(unsigned, int);
static void create_pidfile(char *, pid_t);

#if defined(_WIN32)
static void loc_NTInit(void);
#endif

/*
 * Lock to synchronize player threads with update.
 * Update holds it exclusive, commands hold it shared.
 */
empth_rwlock_t *update_lock;
/*
 * Lock to synchronize player threads with shutdown.
 * Shutdown holds it exclusive, player threads in state PS_PLAYING
 * hold it shared.
 */
empth_rwlock_t *shutdown_lock;

static char pidfname[] = "server.pid";

/* Run as daemon?  If yes, detach from controlling terminal etc. */
static int daemonize = 1;

static void
help(char *program_name, char *complaint)
{
    if (complaint)
	fprintf(stderr, "%s: %s\n", program_name, complaint);
    fprintf(stderr, "Try -h for help.\n");
}

static void
print_usage(char *program_name)
{
    printf("Usage: %s [OPTION]...\n"
	   "  -d              debug mode, implies -E abort\n"
	   "  -e CONFIG-FILE  configuration file\n"
	   "                  (default %s)\n"
	   "  -E ACTION       what to do on oops: abort, crash-dump, nothing (default)\n"
#ifdef _WIN32
	   "  -i              install service `%s'\n"
	   "  -I NAME         install service NAME\n"
#endif
	   "  -p              threading debug mode, implies -d\n"
#ifdef _WIN32
	   "  -u              uninstall service `%s'\n"
	   "  -U NAME         uninstall service NAME\n"
#endif
	   "  -s              enable stack checking\n"
	   "  -R RANDOM-SEED  random seed\n"
	   "  -h              display this help and exit\n"
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
    static char *oops_key[] = { "abort", "crash-dump", "nothing", NULL };
    static void (*oops_hndlr[])(void) = { abort, crash_dump, ignore };
    int flags = 0;
#if defined(_WIN32)
    int install_service_set = 0;
    char *program_name = NULL;
    char *service_name = NULL;
    int remove_service_set = 0;
#endif
    char *config_file = NULL;
    int force_bad_state = 0;
    int op, idx, sig;
    unsigned seed = time(NULL);

    oops_handler = ignore;

#ifdef _WIN32
# define XOPTS "iI:uU:"
#else
# define XOPTS
#endif
    while ((op = getopt(argc, argv, "de:E:FhpsR:v" XOPTS)) != EOF) {
	switch (op) {
	case 'p':
	    flags |= EMPTH_PRINT;
	    /* fall through */
	case 'd':
	    oops_handler = abort;
	    daemonize = 0;
	    break;
	case 'e':
	    config_file = optarg;
	    break;
	case 'E':
	    idx = stmtch(optarg, oops_key, 0, sizeof(*oops_key));
	    if (idx < 0) {
		help(argv[0], "invalid argument for -E");
		return EXIT_FAILURE;
	    }
	    oops_handler = oops_hndlr[idx];
	    break;
	case 'F':
	    force_bad_state = 1;
	    break;
#if defined(_WIN32)
	case 'I':
	    service_name = optarg;
	    /* fall through */
	case 'i':
	    install_service_set++;
	    break;
	case 'U':
	    service_name = optarg;
	    /* fall through */
	case 'u':
	    remove_service_set++;
	    break;
#endif	/* _WIN32 */
	case 's':
	    flags |= EMPTH_STACKCHECK;
	    break;
	case 'R':
	    seed = strtoul(optarg, NULL, 10);
	    break;
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    return EXIT_SUCCESS;
	case 'h':
	    print_usage(argv[0]);
	    return EXIT_SUCCESS;
	default:
	    help(argv[0], NULL);
	    return EXIT_FAILURE;
	}
    }

    /* silently ignore operands for backward compatibility */

#if defined(_WIN32)
    if ((!daemonize || flags || config_file != NULL)
	&& remove_service_set) {
	fprintf(stderr, "Can't use -p, -s, -d or -e with either "
	    "-u or -U options\n");
	exit(EXIT_FAILURE);
    }
    if ((!daemonize || flags) && install_service_set) {
	fprintf(stderr,
		"Can't use -d, -p or -s with either -i or -I options\n");
	exit(EXIT_FAILURE);
    }
    if (install_service_set && remove_service_set) {
	fprintf(stderr, "Can't use both -u or -U and -i or -I options\n");
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

    empfile_init();
    if (emp_config(config_file) < 0)
	exit(EXIT_FAILURE);
    empfile_fixup();
    if (read_builtin_tables() < 0)
	exit(EXIT_FAILURE);
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

    init_server(seed, force_bad_state);

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
	    /* if you make changes here, also update relo() */
	    journal_reopen();
	    update_reschedule();
	    logreopen();
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

static void
ignore(void)
{
}

static void
crash_dump(void)
{
#ifdef _WIN32
    logerror("Crash dump is not implemented");
#else
    pid_t pid;
    int status;

    fflush(NULL);
    pid = fork();
    if (pid < 0) {
	logerror("Can't fork for crash dump (%s)", strerror(errno));
	return;
    }
    if (pid == 0)
	raise(SIGABRT);		/* child */

    /* parent */
    while (waitpid(pid, &status, 0) < 0) {
	if (errno != EINTR) {
	    logerror("Can't get crash dumping child's status (%s)",
		     strerror(errno));
	    return;
	}
    }
    run_hook(post_crash_dump_hook, "post-crash-dump");
    logerror("Crash dump complete");
#endif
}

/*
 * Initialize for serving, acquire resources.
 */
static void
init_server(unsigned seed, int force_bad_state)
{
    srandom(seed);
#if defined(_WIN32)
    loc_NTInit();
#endif
    player_init();
    ef_init_srv(force_bad_state);
    io_init();
    init_nreport();

    if (journal_startup() < 0)
	exit(1);
    journal_prng(seed);
    loginit("server");
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

    update_lock = empth_rwlock_create("Update");
    shutdown_lock = empth_rwlock_create("Shutdown");
    if (!update_lock || !shutdown_lock)
	exit_nomem();

    market_init();
    update_init();
    empth_create(player_accept, 50 * 1024, flags, "AcceptPlayers", NULL);
}

/*
 * Finish serving, release resources.
 */
void
finish_server(void)
{
    ef_fin_srv();
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
    time_t now = time(NULL);
    int i;

    logerror("Shutdown commencing (cleaning up threads.)");

    for (p = player_next(NULL); p; p = player_next(p)) {
	if (p->state != PS_PLAYING)
	    continue;
	pr_flash(p, "Server shutting down...\n");
	io_set_eof(p->iop);
	p->aborted = 1;
	p->may_sleep = PLAYER_SLEEP_NEVER;
	if (p->command) {
	    pr_flash(p, "Shutdown aborting command\n");
	}
	empth_wakeup(p->proc);
    }

    empth_rwlock_wrlock(shutdown_lock);
    empth_yield();

    for (i = 1; i <= 3 && player_next(NULL); i++) {
	logerror("Waiting for player threads to terminate\n");
	empth_sleep(now + i);
    }

    for (p = player_next(NULL); p; p = player_next(p))
	logerror("Player %d lingers, output might be lost", p->cnum);

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

    rc = w32_socket_init();
    if (rc != 0) {
	logerror("WSAStartup Failed, error code %d\n", rc);
	exit(1);
    }
}
#endif
