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
 *  expect.c: Read from the socket, expecting to see a particular code.
 * 
 *  Known contributors to this file:
 *      Steve McClure, 1998
 *      Markus Armbruster, 2007
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include "misc.h"
#include "proto.h"

#ifdef _WIN32
#define read(sock, buffer, buf_size) \
	w32_recv((sock), (buffer), (buf_size), 0)
#define write(sock, buffer, buf_size) \
	w32_send((sock), (buffer), (buf_size), 0)
#endif

int
recvline(int s, char *buf)
{
    int sz = 1024;
    char *bp, *end;
    char ch;
    ssize_t n;
    long id;

    bp = buf;
    for (;;) {
	n = read(s, &ch, 1);
	if (n < 0) {
	    if (errno != EINTR) {
		perror("read");
		exit(1);
	    }
	    continue;
	}
	if (n == 0)
	    return -1;
	if (ch == '\n')
	    break;
	if (bp < buf + sz - 2)
	    *bp++ = ch;
	/* else silently truncate */
    }

    *bp++ = ch;
    *bp = 0;

    id = strtol(buf, &end, 16);
    if (end == buf || *end != ' ') {
	fprintf(stderr, "Malformed id in line %s", buf);
	id = -1;
    }
    if (id > C_LAST)
	id = -1;
    return id;
}

int
expect(int s, int match, char *buf)
{
    int code = recvline(s, buf);
    return code == match;
}

void
sendcmd(int s, char *cmd, char *arg)
{
    char buf[128];
    char *p;
    ssize_t n;
    int len;

    len = snprintf(buf, sizeof(buf), "%s %s\n",
		   cmd, arg != NULL ? arg : "");
    if (len >= (int)sizeof(buf)) {
	fprintf(stderr, "%s too long\n", cmd);
	exit(1);
    }
    p = buf;
    while (len > 0) {
	n = write(s, buf, len);
	if (n < 0) {
	    if (errno != EINTR) {
		perror("sendcmd: write");
		exit(1);
	    }
	    n = 0;
	}
	p += n;
	len -= n;
    }
}
