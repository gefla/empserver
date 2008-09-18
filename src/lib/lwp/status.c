/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1994-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                           Ken Stevens, Steve McClure
 *  Copyright (C) 1991-3 Stephen Crane
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
 *  status.c: Process and perhaps display status messages
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include "lwp.h"
#include "lwpint.h"

void
lwpStatus(struct lwpProc *proc, char *format, ...)
{
    va_list ap;
    static struct timeval startTime;
    struct timeval tv;
    int sec, msec;

    va_start(ap, format);
    if (proc->flags & LWP_PRINT) {
	if (startTime.tv_sec == 0)
	    gettimeofday(&startTime, 0);
	gettimeofday(&tv, 0);
	sec = tv.tv_sec - startTime.tv_sec;
	msec = (tv.tv_usec - startTime.tv_usec) / 1000;
	if (msec < 0) {
	    sec++;
	    msec += 1000;
	}
	printf("%d:%02d.%03d %17s[%d]: ",
	       sec / 60, sec % 60, msec / 10, proc->name, proc->pri);
	vprintf(format, ap);
	putchar('\n');
    }
    va_end(ap);
}
