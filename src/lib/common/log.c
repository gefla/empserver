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
 *  log.c: Log an Empire error to a file
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include <errno.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "deity.h"
#include "player.h"
#include "common.h"
#include "optlist.h"

static s_char *logfile = 0;

s_char *
getlogfile(void)
{
    return (s_char *)logfile;
}

/*
 * Points logfile at datadir/"program".log
 */
void
loginit(void)
{
    extern s_char program[];
    s_char buf[1024];

#if !defined(_WIN32)
    sprintf(buf, "%s/%s.log", datadir, program);
#else
    sprintf(buf, "%s\\%s.log", datadir, program);
#endif
    logfile = malloc(strlen(buf) + 1);
    strcpy(logfile, buf);
}

/*VARARGS*/
void
logerror(s_char *format, ...)
{
    va_list list;
    time_t now;
    s_char buf[512];
    s_char cbuf[512];
    s_char buf1[512];
    int logf;
    s_char *p;

    if (logfile == 0)
	loginit();
    va_start(list, format);
    vsprintf(buf, format, list);
    if ((p = strchr(buf, '\n')) != 0)
	*p = 0;
    (void)time(&now);
    strcpy(cbuf, ctime(&now));
    if ((p = strchr(cbuf, '\n')) != 0)
	*p = 0;
    (void)sprintf(buf1, "%s %s\n", cbuf, buf);
    if ((logf = open(logfile, O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0)
	return;
    (void)write(logf, buf1, strlen(buf1));
    (void)close(logf);
    errno = 0;
#ifdef notdef
    if (player) {
	pr("A system error has occured; please notify the deity.\n");
	pr(buf1);
    }
#endif
    va_end(list);
}
