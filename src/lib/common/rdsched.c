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
 *  rdsched.c: Read update schedule
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#define _XOPEN_SOURCE 500

#include <config.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "prototypes.h"

static int parse_schedule_line(char *, time_t[], int, time_t, time_t *,
			       char *, int);
static int time_ok(time_t, char *, int);
static char *parse_time(time_t *, char *, time_t *);
static char *parse_every(time_t *, char *);
static char *parse_until(time_t *, char *, time_t *);
static char *parse_skip(time_t *, char *, time_t *);
static int insert_update(time_t, time_t[], int, time_t);
static int delete_update(time_t, time_t[], int);

/*
 * Read update schedule from file FNAME.
 * Put the first N-1 updates after T0 into SCHED[] in ascending order,
 * terminated with a zero.
 * Use ANCHOR as initial anchor for anchor-relative times. 
 * Return 0 on success, -1 on failure.
 */
int
read_schedule(char *fname, time_t sched[], int n, time_t t0, time_t anchor)
{
    FILE *fp;
    int lno = 0;
    char buf[1024];
    char *endp;

    if (fname) {
	fp = fopen(fname, "r");
	if (!fp) {
	    logerror("Can't open %s for reading (%s)\n",
		     fname, strerror(errno));
	    return -1;
	}
    } else {
	fp = stdin;
	fname = "<stdin>";
    }

    sched[0] = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
	++lno;
	endp = strchr(buf, '#');
	if (endp)
	    *endp = 0;
	if (parse_schedule_line(buf, sched, n, t0, &anchor, fname, lno))
	    return -1;
    }

    fclose(fp);
    return 0;
}

/*
 * Parse an update schedule directive from LINE.
 * Update SCHED[] and ANCHOR accordingly.
 * SCHED[] holds the first N-1 updates after T0 in ascending order.
 * FNAME and LNO file name and line number for reporting errors.
 */
static int
parse_schedule_line(char *line, time_t sched[], int n,
		    time_t t0, time_t *anchor,
		    char *fname, int lno)
{
    char *endp, *p;
    int bol;
    time_t t, delta, u;

    if ((endp = parse_time(&t, line, anchor))) {
	if (!time_ok(t, fname, lno))
	    return -1;
	*anchor = t;
	insert_update(t, sched, n, t0);
    } else if ((endp = parse_every(&delta, line))) {
	if ((p = parse_until(&u, endp, anchor))) {
	    endp = p;
	    if (!time_ok(u, fname, lno))
		return -1;
	} else
	    u = (time_t)-1;
	t = *anchor;
	do {
	    t += delta;
	} while ((u == (time_t)-1 || t <= u)
		 && insert_update(t, sched, n, t0) < n - 1);
    } else if ((endp = parse_skip(&t, line, anchor))) {
	if (!time_ok(t, fname, lno))
	    return -1;
	delete_update(t, sched, n);
    } else
	endp = line;

    bol = endp == line;
    while (isspace(*endp)) endp++;
    if (*endp) {
	if (bol)
	    logerror("%s:%d: unintelligible\n", fname, lno);
	else
	    logerror("%s:%d: trailing junk\n", fname, lno);
	return -1;
    }

    return 0;
}

/*
 * Complain and return zero when T is bad, else return non-zero.
 * FNAME and LNO file name and line number.
 */
static int
time_ok(time_t t, char *fname, int lno)
{
    if (t == (time_t)-1) {
	logerror("%s:%d: time weird\n", fname, lno);
	return 0;
    }
    return 1;
}

/*
 * Parse a time from S into *T.
 * *ANCHOR is the base for anchor-relative time.
 * Return pointer to first character not parsed on success,
 * null pointer on failure.
 */
static char *
parse_time(time_t *t, char *s, time_t *anchor)
{
    static char *fmt[] = {
	"%Y-%m-%d %H:%M ",	/* ISO 8601 */
	"%b %d %H:%M %Y ",	/* like ctime(): Dec 22 15:35 2006 */
	"%d %b %Y %H:%M ",	/* 22 Dec 2006 15:35 */
	"next %a %H:%M ",	/* next Fri 15:35 */
	"next %a ",		/* next Fri */
	NULL
    };
    char *p, *endp;
    int i;
    struct tm tm, nexttm;

    for (p = s; isspace(*(unsigned char *)p); ++p) ;

    for (i = 0; ; i++) {
	if (!fmt[i])
	    return NULL;
	memset(&tm, 0, sizeof(tm));
	tm.tm_hour = -1;
	endp = strptime(p, fmt[i], &tm);
	if (endp)
	    break;
    }

    if (tm.tm_mday == 0) {
	/* relative to anchor */
	nexttm = *localtime(anchor);
	if (tm.tm_hour >= 0) {
	    /* got hour and minute */
	    nexttm.tm_hour = tm.tm_hour;
	    nexttm.tm_min = tm.tm_min;
	    nexttm.tm_sec = 0;
	}
	nexttm.tm_mday += tm.tm_wday - nexttm.tm_wday;
	if (tm.tm_wday <= nexttm.tm_wday)
	    nexttm.tm_mday += 7;
	tm = nexttm;
    }

    *t = mktime(&tm);
    return endp;
}

/*
 * Parse an every clause from S into *SECS.
 * Return pointer to first character not parsed on success,
 * null pointer on failure.
 */
static char *
parse_every(time_t *secs, char *s)
{
    int nch, delta;

    nch = -1;
    sscanf(s, " every %u hours%n", &delta, &nch);
    if (nch >= 0)
	delta *= 60;
    else
	sscanf(s, " every %u minutes%n", &delta, &nch);
    if (nch < 0)
	return NULL;    *secs = 60 * delta;
    return s + nch;
}

/*
 * Parse an until clause from S into *T.
 * *ANCHOR is the base for anchor-relative time.
 * Return pointer to first character not parsed on success,
 * null pointer on failure.
 */
static char *
parse_until(time_t *t, char *s, time_t *anchor)
{
    int nch;

    nch = -1;
    sscanf(s, " until%n", &nch);
    if (nch < 0)
	return NULL;
    return parse_time(t, s + nch, anchor);
}

/*
 * Parse an skip clause from S into *T.
 * *ANCHOR is the base for anchor-relative time.
 * Return pointer to first character not parsed on success,
 * null pointer on failure.
 */
static char *
parse_skip(time_t *t, char *s, time_t *anchor)
{
    int nch;

    nch = -1;
    sscanf(s, " skip%n", &nch);
    if (nch < 0)
	return NULL;
    return parse_time(t, s + nch, anchor);
}

/*
 * Return the index of the first update at or after T in SCHED[].
 */
static int
find_update(time_t t, time_t sched[])
{
    int i;

    /* Could use binary search here, but it's hardly worth it */
    for (i = 0; sched[i] && t > sched[i]; i++) ;
    return i;
}

/*
 * Insert update at T into SCHED[].
 * SCHED[] holds the first N-1 updates after T0 in ascending order.
 * If T is before T0 or outside game_days/game_hours, return -1.
 * If there's no space for T in SCHED[], return N-1.
 * Else insert T into SCHED[] and return its index in SCHED[].
 */
static int
insert_update(time_t t, time_t sched[], int n, time_t t0)
{
    int i;

    if (t <= t0 || !gamehours(t))
	return -1;

    i = find_update(t, sched);
    memmove(sched + i + 1, sched + i, (n - 1 - i) * sizeof(*sched));
    sched[i] = t;
    sched[n - 1] = 0;
    return i;
}

/*
 * Delete update T from SCHED[].
 * SCHED[] holds N-1 updates in ascending order.
 * Return the index of the first update after T in SCHED[].
 */
static int
delete_update(time_t t, time_t sched[], int n)
{
    int i = find_update(t, sched);
    if (t == sched[i])
	memmove(sched + i, sched + i + 1,
		(n - 1 - i) * sizeof(*sched));
    return i;
}
