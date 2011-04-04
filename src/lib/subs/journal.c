/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
 *                Ken Stevens, Steve McClure, Markus Armbruster
 *
 *  Empire is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  journal.c: Log a journal of events to a file
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2011
 *     Ron Koenderink, 2008
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
 *     prng NAME SEED
 *     login CNUM HOSTADDR USER
 *     logout CNUM
 *     command NAME
 *     input INPUT
 *     output THREAD ID OUTPUT
 *     update ETU
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "misc.h"
#include "empthread.h"
#include "journal.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

static char journal_fname[] = "journal.log";
static FILE *journal;

static void journal_entry_start(char *fmt, ...)
    ATTRIBUTE((format (printf, 1, 2)));
static void journal_entry(char *fmt, ...)
    ATTRIBUTE((format (printf, 1, 2)));
static void journal_output_start(struct player *, int);

static FILE *
journal_open(void)
{
    return fopen(journal_fname, "a+");
}

static void
journal_entry_vstart(char *fmt, va_list ap)
{
    time_t now;

    if (!journal)
	return;
    time(&now);
    fprintf(journal, "%.24s %10.10s ",
	    ctime(&now), empth_name(empth_self()));
    vfprintf(journal, fmt, ap);
}

static void
journal_entry_start(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    journal_entry_vstart(fmt, ap);
    va_end(ap);
}

static void
journal_entry_pr(char *s, size_t n)
{
    unsigned char *p;

    if (!journal)
	return;
    for (p = (unsigned char *)s; *p && n; p++) {
	if (*p == '\\')
	    fputs("\\\\", journal);
	else if (isprint(*p))
	    putc(*p, journal);
	else
	    fprintf(journal, "\\%03o", *p);
	n--;
    }
}

static void
journal_entry_end(int newline, int flush)
{
    if (!journal)
	return;
    if (!newline)
	fputc('\\', journal);
    fputc('\n', journal);
    fflush(journal);
    if (ferror(journal)) {
	logerror("Error writing journal (%s)", strerror(errno));
	clearerr(journal);
    }
}

static void
journal_entry(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    journal_entry_vstart(fmt, ap);
    va_end(ap);
    journal_entry_end(1, 1);
}

int
journal_startup(void)
{
    if (!keep_journal)
	return 0;
    journal = journal_open();
    if (!journal) {
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
    if (journal) {
	fclose(journal);
	journal = NULL;
    }
}

int
journal_reopen(void)
{
    FILE *j;

    if (!keep_journal)
	return 0;
    j = journal_open();
    if (!j) {
	logerror("Can't open %s (%s)", journal_fname, strerror(errno));
	return -1;
    }
    if (journal)
	fclose(journal);
    journal = j;
    return 0;
}

void
journal_prng(unsigned seed)
{
    journal_entry("prng BSD %d", seed);
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
journal_output(struct player *pl, int id, char *output)
{
    static char buf[1024];
    static char *bp = buf;
    static struct player *bpl;
    static int bid;
    char *s, *e;

    if (keep_journal < 2)
	return;

    if (bp != buf && (pl != bpl || id != bid)) {
	journal_output_start(bpl, bid);
	journal_entry_pr(buf, bp - buf);
	journal_entry_end(0, 0);
	bp = buf;
    }

    for (s = output; (e = strchr(s, '\n')); s = e + 1) {
	journal_output_start(pl, id);
	journal_entry_pr(buf, bp - buf);
	journal_entry_pr(s, e - s);
	journal_entry_end(1, 0);
	bp = buf;
    }
    e = strchr(s, 0);
    if (bp + (e - s) <= buf + sizeof(buf)) {
	memcpy(bp, s, e - s);
	bp += e - s;
	bpl = pl;
	bid = id;
    } else {
	journal_output_start(pl, id);
	journal_entry_pr(buf, bp - buf);
	journal_entry_pr(s, e - s);
	journal_entry_end(0, 0);
	bp = buf;
    }
}

static void
journal_output_start(struct player *pl, int id)
{
    journal_entry_start("output %s %d ", empth_name(pl->proc), id);
}

void
journal_input(char *input)
{
    journal_entry_start("input ");
    journal_entry_pr(input, -1);
    journal_entry_end(1, 1);
}

void
journal_command(char *cmd)
{
    char *eptr = strchr(cmd, ' ');
    journal_entry("command %.*s", eptr ? (int)(eptr - cmd) : -1, cmd);
}

void
journal_update(int etu)
{
    journal_entry("update %d", etu);
}
