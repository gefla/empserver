/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  main.c: client main function
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 *     Ron Koenderink, 2004-2007
 *     Markus Armbruster, 2005-2009
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include "sys/socket.h"
#else
#include <pwd.h>
#endif
#include <unistd.h>
#include "misc.h"
#include "version.h"

#ifdef _WIN32
#define getuid() 0
#define getpwuid(uid) ((void)(uid), w32_getpw())
#define sysdep_init() w32_sysdep_init()

struct passwd {
    char *pw_name;
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
	   "  -s HOST:PORT    Set host and port to connect\n"
	   "  -u              Use UTF-8\n"
	   "  -h              display this help and exit\n"
	   "  -v              display version information and exit\n",
	   program_name);
}

int
main(int argc, char **argv)
{
    int opt;
    char *auxfname = NULL;
    int send_kill = 0;
    char *host = NULL;
    char *port = NULL;
    int utf8 = 0;
    char **ap;
    char *country;
    char *passwd;
    char *uname;
    int sock;

    while ((opt = getopt(argc, argv, "2:ks:uhv")) != EOF) {
	switch (opt) {
	case '2':
	    auxfname = optarg;
	    break;
	case 'k':
	    send_kill = 1;
	    break;
	case 's':
	    host = strdup(optarg);
	    port = strchr(host, ':');
	    if (port == host) { /* if no host specified, then set to null */
		host = NULL;
	    }
	    if (port) {	       /* make port the bit after the colon */
		port[0] = 0;
		port++;
		if (port[0] == 0) { /* handle colon-at-end-of-string */
		    port = NULL;
		}
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
    if (uname == NULL) {
	struct passwd *pwd;

	pwd = getpwuid(getuid());
	if (pwd == NULL) {
	    fprintf(stderr, "You don't exist.  Go away\n");
	    exit(1);
	}
	uname = pwd->pw_name;
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

    if (!login(sock, uname, country, passwd, send_kill, utf8))
	exit(1);

    if (play(sock) < 0)
	exit(1);

    return 0;
}

#ifdef _WIN32
/*
 * Get Windows user name
 */
static struct passwd *
w32_getpw(void)
{
    static char unamebuf[128];
    static struct passwd pwd;
    DWORD unamesize;

    unamesize = sizeof(unamebuf);
    if (GetUserName(unamebuf, &unamesize)) {
	pwd.pw_name = unamebuf;
	if (unamesize == 0 || strlen(unamebuf) == 0)
	    pwd.pw_name = "nobody";
    } else
	pwd.pw_name = "nobody";
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
