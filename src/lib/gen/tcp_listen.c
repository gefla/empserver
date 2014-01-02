/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  tcp_listen.c: Create a socket and listen on it
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2005-2013
 */

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "prototypes.h"

/* Portability cruft, should become unnecessary eventually */
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

static void cant_listen(char *, char *, const char *)
    ATTRIBUTE((noreturn));

int
tcp_listen(char *host, char *serv, size_t *addrlenp)
{
    int fd;
    int on = 1;
#ifdef HAVE_GETADDRINFO
    /*
     * Inspired by example code from W. Richard Stevens: UNIX Network
     * Programming, Vol. 1
     */
    int err;
    struct addrinfo hints, *first_ai, *ai;
    /* Crap necessary for OpenBSD, see below */
    int try_v6only_off = 1, v6only_stuck = 0;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(host, serv, &hints, &first_ai)) != 0)
	cant_listen(host, serv, gai_strerror(err));
    assert(first_ai);

again:
    for (ai = first_ai; ai; ai = ai->ai_next) {
	fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (fd < 0)
	    continue;		/* error, try next one */

#ifdef IPV6_V6ONLY
	if (ai->ai_family == AF_INET6 && try_v6only_off) {
	    int off = 0;

	    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
			   &off, sizeof(off)) < 0) {
		/*
		 * IPV6_V6ONLY is stuck on, violating RFC 3493 (gee,
		 * thanks, OpenBSD!).  Address is good only for IPv6,
		 * not for IPv4.  Means we can't have both on this
		 * system.  Continue looking for one that's good for
		 * IPv4.
		 */
		v6only_stuck = 1;
		close(fd);
		continue;
	    }
	}
#else
	(void)try_v6only_off;
#endif

	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0)
	    break;		/* success */

	close(fd);		/* error, close and try next one */
    }

    /* More crap for OpenBSD */
    if (ai == NULL && v6only_stuck) {
	/*
	 * No go.  But we skipped IPv6 addresses that don't work for
	 * IPv4, but could for IPv6.  Try again without skipping
	 * these.
	 */
	try_v6only_off = 0;
	goto again;
    }

    if (ai == NULL)	     /* errno from final socket() or bind() */
	cant_listen(host, serv, strerror(errno));

    if (listen(fd, SOMAXCONN) < 0)
	cant_listen(host, serv, strerror(errno));

    if (addrlenp)
	*addrlenp = ai->ai_addrlen;

    freeaddrinfo(first_ai);

#else  /* !HAVE_GETADDRINFO */
    struct sockaddr_in sin;
    struct hostent *hp;
    struct servent *sp;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    if (!host)
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
    else if (isdigit(*host))
	sin.sin_addr.s_addr = inet_addr(host);
    else {
	hp = gethostbyname(host);
	if (!hp)
	    cant_listen(host, serv, strerror(errno));
	memcpy(&sin.sin_addr, hp->h_addr, sizeof(sin.sin_addr));
    }
    if (isdigit(*serv))
	sin.sin_port = htons(atoi(serv));
    else {
	sp = getservbyname(serv, "tcp");
	if (!sp)
	    cant_listen(host, serv, strerror(errno));
	sin.sin_port = sp->s_port;
    }
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	cant_listen(host, serv, strerror(errno));
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	cant_listen(host, serv, strerror(errno));
    if (listen(fd, SOMAXCONN) < 0)
	cant_listen(host, serv, strerror(errno));

    if (addrlenp)
	*addrlenp = sizeof(struct sockaddr_in);

#endif /* !HAVE_GETADDRINFO */

    return fd;
}

static void
cant_listen(char *host, char *serv, const char *err)
{
    fprintf(stderr, "Can't listen on %s%s%s: %s\n",
	    host ? host : "", host ? ":" : "", serv, err);
    exit(1);
}
