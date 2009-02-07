/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Ron Koenderink, 2004-2005
 *     Markus Armbruster, 2005-2008
 */

#include <config.h>

#ifndef _WIN32
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#endif
#include "misc.h"
#include "version.h"

#ifdef _WIN32
#define getuid() 0
#define getpwuid(uid) ((void)(uid), w32_getpw())
#else
#define sysdep_init() ((void)0)
#endif

static void
print_usage(char *program_name)
{
    printf("Usage: %s [OPTION]...[COUNTRY [PASSWORD]]\n"
	   "  -2 FILE         Append log of session to FILE\n"
	   "  -k              Kill connection\n"
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
    int utf8 = 0;
    char **ap;
    char *country;
    char *passwd;
    char *uname;
    char *host;
    char *port;
    int sock;

    while ((opt = getopt(argc, argv, "2:kuhv")) != EOF) {
	switch (opt) {
	case '2':
	    auxfname = optarg;
	    break;
	case 'k':
	    send_kill = 1;
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
	    print_usage(argv[0]);
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
    port = getenv("EMPIREPORT");
    if (!port)
	port = empireport;
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
