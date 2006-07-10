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
 *  log.c: Log an Empire error to a file
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include <config.h>

#include "misc.h"
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

/* Debugging?  If yes call abort() on internal error.  */
int debug = 0;

static char logfile[32];

/*
 * Points log file at PROGRAM.log
 */
void
loginit(char *program)
{
    sprintf(logfile, "%.*s.log", (int)sizeof(logfile) - 5, program);
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
    int logf;
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
    time(&now);
    memcpy(buf, ctime(&now), ctime_len);
    buf[ctime_len] = ' ';
    if ((logf = open(logfile, O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0)
	return;
    write(logf, buf, strlen(buf));
    close(logf);
    va_end(list);
}

/*
 * Log internal error MSG occured in FILE:LINE.
 * If debugging, call abort(), else return 1.
 */
int
oops(char *msg, char *file, int line)
{
  logerror("Oops: %s in %s:%d", msg ? msg : "bug", file, line);
  if (debug) abort();
  return 1;
}
