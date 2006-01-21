/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  host.c: make stream connection to empire
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 *     Markus Armbruster, 2005
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <io.h>
#include <winsock.h>
#endif
#include "misc.h"

#ifdef HAVE_GETADDRINFO
/*
 * Inspired by example code from W. Richard Stevens: UNIX Network
 * Programming, Vol. 1
 */

int
tcp_connect(char *host, char *serv)
{
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
	fprintf(stderr, "Can't connect to %s:%s: %s\n",
		host, serv, gai_strerror(n));
	exit(1);
    }
    ressave = res;

    do {
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0)
	    continue;		/* ignore this one */

	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
	    break;		/* success */

	close(sockfd);		/* ignore this one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {		/* errno set from final connect() */
	fprintf(stderr, "Can't connect to %s:%s: %s\n",
		host, serv, strerror(errno));
	exit(1);
    }

    freeaddrinfo(ressave);

    return sockfd;
}

#else  /* !HAVE_GETADDRINFO */

static int
hostaddr(char *name, struct sockaddr_in *addr)
{
    struct hostent *hp;

    if (name == NULL || *name == 0)
	return 0;
    if (isdigit(*name)) {
	addr->sin_addr.s_addr = inet_addr(name);
    } else {
	hp = gethostbyname(name);
	if (hp == NULL)
	    return 0;
	memcpy(&addr->sin_addr, hp->h_addr, sizeof(addr->sin_addr));
    }
    return 1;
}

static int
hostport(char *name, struct sockaddr_in *addr)
{
    struct servent *sp;

    if (name == NULL || *name == 0)
	return 0;
    if (isdigit(*name)) {
	addr->sin_port = htons(atoi(name));
    } else {
	sp = getservbyname(name, "tcp");
	if (sp == NULL)
	    return 0;
	addr->sin_port = sp->s_port;
    }
    return 1;
}

static int
hostconnect(struct sockaddr_in *addr)
{
    /* FIXME should attempt connect to all addresses of multi-homed host, not just 1st */
    int s;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	return -1;
    }
    addr->sin_family = AF_INET;
    if (connect(s, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	(void)close(s);
	return -1;
    }
    return s;
}

int
tcp_connect(char *host, char *serv)
{
    struct sockaddr_in sin;
    int sock;

    if (!hostport(serv, &sin)) {
	fprintf(stderr, "Can't resolve Empire port %s\n", serv);
	exit(1);
    }
    if (!hostaddr(host, &sin)) {
	fprintf(stderr, "Can't resolve Empire host %s\n", host);
	exit(1);
    }
    if ((sock = hostconnect(&sin)) < 0) {
	fprintf(stderr, "Can't connect to %s:%s: %s\n",
		serv, host, strerror(errno));
	exit(1);
    }
    return sock;
}
#endif
