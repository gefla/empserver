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
 *  host.c: make stream connection to empire
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1998
 */

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
#include <winsock.h>
#endif
#include <ctype.h>
#include "misc.h"

int
hostaddr(name, addr)
s_char *name;
struct sockaddr_in *addr;
{
    struct hostent *hp;

    if (name == 0 || *name == 0)
	return 0;
    if (isdigit(*name)) {
	addr->sin_addr.s_addr = inet_addr(name);
    } else {
	hp = gethostbyname(name);
	if (hp == NULL) {
	    fprintf(stderr, "%s: No such host\n", name);
	    return 0;
	}
	memcpy(&addr->sin_addr, hp->h_addr, sizeof(addr->sin_addr));
#ifdef _WIN32
	printf("Trying to connect to '%s'\n", inet_ntoa(addr->sin_addr));
	fflush(stdout);
#endif
    }
    return 1;
}

int
hostport(name, addr)
s_char *name;
struct sockaddr_in *addr;
{
    struct servent *sp;

    if (name == 0 || *name == 0)
	return 0;
    if (isdigit(*name)) {
#ifndef _WIN32
	addr->sin_port = htons(atoi(name));
#else
	addr->sin_port = atoi(name);
	addr->sin_port = htons(addr->sin_port);
#endif
    } else {
	sp = getservbyname(name, "tcp");
	if (sp == NULL)
	    return 0;
	addr->sin_port = sp->s_port;
    }
    return 1;
}

int
hostconnect(addr)
struct sockaddr_in *addr;
{
    int s;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	perror("socket");
	return -1;
    }
    addr->sin_family = AF_INET;
    if (connect(s, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	perror("connect");
#ifdef _WIN32
	printf("Check that your EMPIREHOST and EMPIREPORT are correct.\n");
#endif
	(void)close(s);
	return -1;
    }
    return s;
}
