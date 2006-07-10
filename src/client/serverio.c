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
 *  serverio.c: Handle input from server
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "misc.h"
#include "ioqueue.h"

int
serverio(int s, struct ioqueue *ioq)
{
    char *buf;
    int n;

    if ((buf = malloc(ioq->bsize)) == NULL) {
	fprintf(stderr, "malloc server i/o failed\n");
	return 0;
    }
#ifdef _WIN32
    n = recv(s, buf, ioq->bsize, 0);
#else
    n = read(s, buf, ioq->bsize);
#endif
    if (n < 0) {
#ifdef _WIN32
	errno = WSAGetLastError();
#endif
	perror("server i/o read");
	free(buf);
	return 0;
    }
    if (n == 0) {
	fprintf(stderr, "Server EOF\n");
	(void)close(s);
	return 0;
    }
    if (n != ioq->bsize)
	buf = realloc(buf, n);
    ioq_write(ioq, buf, n);
    return 1;
}
