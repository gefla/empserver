/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2017, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  main.c: client main function
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 *     Ron Koenderink, 2004-2007
 *     Markus Armbruster, 2005-2015
 *     Tom Dickson-Hunt, 2010
 *     Martin Haukeli, 2015
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include "sys/socket.h"
#else
#include <pwd.h>
#endif
#include <unistd.h>
#include "fnameat.h"
#include "misc.h"
#include "version.h"

#ifdef _WIN32
#define getuid() 0
#define getpwuid(uid) ((void)(uid), w32_getpw())
#define sysdep_init() w32_sysdep_init()

struct passwd {
    char *pw_name;
    char *pw_dir;
};

static struct passwd *w32_getpw(void);
static void w32_sysdep_init(void);
#else  /* !_WIN32 */
#define sysdep_init() ((void)0)
#endif	/* !_WIN32 */

static void
print_usage(char *program_name)
{
    printf("Usage: %s [OPTION]...[COUNTRY [PASSWORD]]\n"
	   "  -2 FILE         Append log of session to FILE\n"
	   "  -k              Kill connection\n"
	   "  -r              Restricted mode, no redirections\n"
	   "  -s [HOST:]PORT  Specify server HOST and PORT\n"
	   "  -u              Use UTF-8\n"
#ifdef HAVE_LIBREADLINE
	   "  -H FILE         Load and save command history from FILE\n"
	   "                  (default ~/.empire_history with -r, none without -r)\n"
#endif /* HAVE_LIBREADLINE */
	   "  -h              display this help and exit\n"
	   "  -v              display version information and exit\n",
	   program_name);
}

int
main(int argc, char **argv)
{
    int opt;
    char *auxfname = NULL;
    char *history_file = NULL;
    int send_kill = 0;
    char *host = NULL;
    char *port = NULL;
    int utf8 = 0;
    char **ap;
    char *country;
    char *passwd;
    char *uname;
    char *udir;
    char *colon;
    int sock;

    while ((opt = getopt(argc, argv, "2:H:krs:uhv")) != EOF) {
	switch (opt) {
	case '2':
	    auxfname = optarg;
	    break;
#ifdef HAVE_LIBREADLINE
	case 'H':
	    history_file = optarg;
	    break;
#endif /* HAVE_LIBREADLINE */
	case 'k':
	    send_kill = 1;
	    break;
	case 'r':
	    restricted = 1;
	    break;
	case 's':
	    port = strdup(optarg);
	    colon = strrchr(port, ':');
	    if (colon) {
		*colon = 0;
		host = port;
		port = colon + 1;
	    }
	    break;
	case 'u':
	    utf8 = eight_bit_clean = 1;
	    break;
	case 'h':
	    print_usage(argv[0]);
	    exit(0);
	case 'v':
	    printf("%s\n\n%s", version, legal);
	    exit(0);
	default:
	    fprintf(stderr, "Try -h for help.\n");
	    exit(1);
	}
    }

    ap = argv + optind;
    if (*ap)
	country = *ap++;
    else
	country = getenv("COUNTRY");
    if (*ap)
	passwd = *ap++;
    else
	passwd = getenv("PLAYER");
    if (!port)
	port = getenv("EMPIREPORT");
    if (!port)
	port = empireport;
    if (!host)
	host = getenv("EMPIREHOST");
    if (!host)
	host = empirehost;
    uname = getenv("LOGNAME");
    udir = getenv("HOME");
    if (!uname || !udir) {
	struct passwd *pwd;

	pwd = getpwuid(getuid());
	if (pwd == NULL) {
	    fprintf(stderr, "You don't exist.  Go away\n");
	    exit(1);
	}
	if (!uname)
	    uname = pwd->pw_name;
	if (!udir)
	    udir = pwd->pw_dir;
    }
    if (*ap) {
	fprintf(stderr, "%s: extra operand %s\n", argv[0], *ap);
	fprintf(stderr, "Try -h for help.\n");
	exit(1);
    }

    getsose();
    if (auxfname && (auxfp = fopen(auxfname, "a")) == NULL) {
	fprintf(stderr, "Unable to open %s for append\n", auxfname);
	exit(1);
    }

    sysdep_init();

    sock = tcp_connect(host, port);

    if (!restricted && !history_file)
	history_file = ".empire_history";
    if (history_file)
	history_file = fnameat(history_file, udir);

    if (!login(sock, uname, country, passwd, send_kill, utf8))
	exit(1);

    if (play(sock, history_file) < 0)
	exit(1);

    return 0;
}

#ifdef _WIN32
/*
 * Get Windows user name and directory
 */
static struct passwd *
w32_getpw(void)
{
    static char unamebuf[128];
    static char udirbuf[MAX_PATH];
    static struct passwd pwd;
    DWORD unamesize;

    unamesize = sizeof(unamebuf);
    if (GetUserName(unamebuf, &unamesize)) {
	pwd.pw_name = unamebuf;
	if (unamesize == 0 || strlen(unamebuf) == 0)
	    pwd.pw_name = "nobody";
    } else
	pwd.pw_name = "nobody";
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, udirbuf))
	&& strlen(udirbuf) == 0)
	pwd.pw_dir = udirbuf;
    return &pwd;
}

static void
w32_sysdep_init(void)
{
    int err;

    /*
     * stdout is unbuffered under Windows if connected to a character
     * device, and putchar() screws up when printing multibyte strings
     * bytewise to an unbuffered stream.  Switch stdout to line-
     * buffered mode.  Unfortunately, ISO C allows implementations to
     * screw that up, and of course Windows does.  Manual flushing
     * after each prompt is required.
     */
    setvbuf(stdout, NULL, _IOLBF, 4096);

    err = w32_socket_init();
    if (err != 0) {
	fprintf(stderr, "WSAStartup Failed, error code %d\n", err);
	exit(1);
    }
}
#endif
