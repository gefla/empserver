/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  expect.c: Read from the socket, expecting to see a particular code.
 * 
 *  Known contributors to this file:
 *      Steve McClure, 1998
 */

#include "misc.h"
#include "fnlist.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock.h>
#endif

int
expect(int s, int match, char *buf)
{
    int size;
    char *p;
    int n;
    int code;
    int newline;
    char *ptr;
    int cc;

    size = 1024;
#ifndef _WIN32
    (void)alarm(30);
#endif
    ptr = buf;
    n = recv(s, ptr, size, MSG_PEEK);
    if (n <= 0) {
	fprintf(stderr, "Expecting code %d\n", match);
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	perror("recv");
	return 0;
    }
    size -= n;
    buf[n] = '\0';
    if ((p = strchr(ptr, '\n')) == NULL) {
	do {
#ifndef _WIN32
	    cc = read(s, ptr, n);
#else
	    cc = recv(s, ptr, n, 0);
#endif
	    if (cc < 0) {
#ifdef _WIN32
		errno = WSAGetLastError();
#endif
		perror("expect: read");
		return 0;
	    }
	    if (cc != n) {
		fprintf(stderr, "expect: short read (%d not %d)\n", cc, n);
		return 0;
	    }
	    ptr += n;
	    if ((n = recv(s, ptr, size, MSG_PEEK)) <= 0) {
		fprintf(stderr, "Expecting %d, got %s\n", match, buf);
		return 0;
	    }
	    size -= n;
	    ptr[n] = '\0';
	} while ((p = strchr(ptr, '\n')) == 0);
	newline = 1 + p - buf;
	*p = 0;
    } else
	newline = 1 + p - ptr;
#ifndef _WIN32
    cc = read(s, buf, newline);
#else
    cc = recv(s, buf, newline, 0);
#endif
    if (cc < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	perror("expect: read #2");
	return 0;
    }
    if (cc != newline) {
	fprintf(stderr, "expect: short read #2 (%d not %d)\n",
		cc, newline);
	return 0;
    }
    buf[newline] = '\0';
#ifndef _WIN32
    (void)alarm(0);
#endif
    if (!isxdigit(*buf)) {
	fprintf(stderr, "Expecting %d, got %s\n", match, buf);
	return 0;
    }
    if (isdigit(*buf))
	code = *buf - '0';
    else {
	if (isupper(*buf))
	    *buf = tolower(*buf);
	code = 10 + *buf - 'a';
    }
    if (code == match)
	return 1;
    return 0;
}

void
sendcmd(int s, int cmd, char *arg)
{
    char buf[128];
    int cc;
    int len;

    (void)sprintf(buf, "%s %s\n", fnlist[cmd].name, arg != NULL ? arg : "");
    len = strlen(buf);
#ifndef _WIN32
    cc = write(s, buf, len);
#else
    cc = send(s, buf, len, 0);
#endif
    if (cc < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	perror("sendcmd: write");
    }
    if (cc != len) {
	fprintf(stderr, "sendcmd: short write (%d not %d)\n", cc, len);
    }
}
