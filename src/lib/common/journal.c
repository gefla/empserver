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
 *  journal.c: Log a journal of events to a file
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2006
 */

/*
 * Journal file format: each line logs an event, and looks like this:
 *
 *     TIME THREAD EVENT DATA
 *
 * Events and their data are:
 *
 *     startup
 *     shutdown
 *     login CNUM HOSTADDR USER
 *     logout CNUM
 *     input INPUT
 *     update ETU
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "misc.h"
#include "empthread.h"
#include "journal.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

static char journal_fname[] = "journal.log";
static FILE *journal;

int
journal_open(void)
{
    journal = fopen(journal_fname, "a+");
    return journal ? 0 : -1;
}

int
journal_close(void)
{
    FILE *j = journal;
    journal = NULL;
    return j ? fclose(j) : 0;
}

void
journal_entry(char *fmt, ...)
{
    static char buf[1024];
    va_list ap;
    time_t now;
    int n, i;

    if (journal) {
	time(&now);
	n = sprintf(buf, "%.24s %p ", ctime(&now), empth_self());
	va_start(ap, fmt);
	vsnprintf(buf + n, sizeof(buf) - n - 1, fmt, ap);
	va_end(ap);

	for (i = n; buf[i]; ++i) {
	    if (!isprint(buf[i]))
		buf[i] = '?';	/* FIXME replace by escape */
	}
	buf[i++] = '\n';
	buf[i] = 0;
	if (fputs(buf, journal) == EOF)
	    logerror("Error writing journal (%s)", strerror(errno));
    }
}

int
journal_startup(void)
{
    if (!keep_journal)
	return 0;
    if (journal_open() < 0) {
	logerror("Can't open %s (%s)", journal_fname, strerror(errno));
	return -1;
    }
    journal_entry("startup");
    return 0;
}

void
journal_shutdown(void)
{
    journal_entry("shutdown");
    journal_close();
}

void
journal_login(void)
{
    journal_entry("login %d %s %s",
		  player->cnum, player->hostaddr, player->userid);
}

void
journal_logout(void)
{
    journal_entry("logout %d", player->cnum);
}

void
journal_input(char *input)
{
    journal_entry("input %s", input);
}

void
journal_update(int etu)
{
    journal_entry("update %d", etu);
}
