/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  log.c: Log an Empire error to a file
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Markus Armbruster, 2003-2008
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "misc.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

void (*oops_handler)(void) = abort;

static char logfile[32];
static int logfd = -1;

static int logopen(void);

/*
 * Points log file at PROGRAM.log
 */
int
loginit(char *program)
{
    sprintf(logfile, "%.*s.log", (int)sizeof(logfile) - 5, program);
    logfd = logopen();
    return logfd;
}

static int
logopen(void)
{
    int fd;

    fd = open(logfile, O_WRONLY | O_CREAT | O_APPEND, S_IRWUG);
    if (fd < 0)
	logerror("Can't open %s (%s)", logfile, strerror(errno));
    return fd;
}

int
logreopen(void)
{
    int newfd, res;

    if ((newfd = logopen()) < 0)
	return -1;
    res = close(logfd);
    logfd = newfd;
    if (res < 0)
	logerror("Can't close %s (%s)", logfile, strerror(errno));
    return res;
}

/*
 * Write a line to the log file and to stderr.
 * Messages are silently truncated after 512 characters or a newline.
 */
void
logerror(char *format, ...)
{
    enum {
	ctime_len = 24,		/* output of ctime() less the newline */
	msg_space = 512		/* space for formatted message */
    };
    va_list list;
    time_t now;
    char buf[ctime_len + 1 + msg_space + 2];
    char *msg, *p;

    va_start(list, format);
    msg = buf + ctime_len + 1;
    vsnprintf(msg, msg_space, format, list);
    buf[sizeof(buf)-2] = 0;
    p = msg + strlen(msg);
    p[0] = '\n';
    p[1] = 0;
    p = strchr(msg, '\n');
    p[1] = 0;
    fputs(msg, stderr);
    if (logfd >= 0) {
	time(&now);
	memcpy(buf, ctime(&now), ctime_len);
	buf[ctime_len] = ' ';
	write(logfd, buf, strlen(buf));
    }
    va_end(list);
}

/*
 * Log internal error MSG occured in FILE:LINE.
 * Call oops handler, and if it returns, return 1.
 * Oops handler defaults to abort().
 */
int
oops(char *msg, char *file, int line)
{
    logerror("Oops: %s in %s:%d", msg ? msg : "bug", file, line);
    oops_handler();
    return 1;
}

/*
 * Report out-of-memory condition and terminate the program.
 * Use this with restraint!  Clean error recovery is preferable, but
 * not always feasible (e.g. halfway through the update) or worthwhile
 * (during server startup).
 */
void
exit_nomem(void)
{
    logerror("Memory exhausted");
    exit(1);
}
