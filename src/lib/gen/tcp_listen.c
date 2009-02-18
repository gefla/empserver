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
 *  tcp_listen.c: Create a socket and listen on it
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2005-2009
 */

#include <config.h>

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

static void cant_listen(char *, char *, const char *);

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
    int n;
    struct addrinfo hints, *res, *ressave;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
	cant_listen(host, serv, gai_strerror(n));
    ressave = res;

    do {
	fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd < 0)
	    continue;		/* error, try next one */

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	    cant_listen(host, serv, strerror(errno));
	if (bind(fd, res->ai_addr, res->ai_addrlen) == 0)
	    break;		/* success */

	close(fd);		/* error, close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL)	     /* errno from final socket() or bind() */
	cant_listen(host, serv, strerror(errno));

    if (listen(fd, SOMAXCONN) < 0)
	cant_listen(host, serv, strerror(errno));

    if (addrlenp)
	*addrlenp = res->ai_addrlen;

    freeaddrinfo(ressave);

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
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	cant_listen(host, serv, strerror(errno));
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
