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
 *  nreport.c: File a news report.  Downgrade relations if things get hostile.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1997
 */

#include "misc.h"
#include "news.h"
#include "nat.h"
#include "deity.h"
#include "file.h"
#include "empio.h"
#include <fcntl.h>
#include "prototypes.h"

static void filereport(int, int, int, int);

void
nreport(natid actor, int event, natid victim, int times)
{
    int nice;
    int rel;
    struct natstr *np;

    filereport(actor, event, victim, times);
    /*
     * this is probably pretty expensive, but hopefully we
     * don't fire zillions of these things off every second.
     */
    if (victim == 0 || (nice = rpt[event].r_good_will) >= 0)
	return;
    /*
     * Pretty schlocky to put it here, but
     * I guess it can't go anywhere else.
     */
    if (actor == victim)
	return;
    if (!chance((double)-nice * times / 20.0))
	return;
    if ((np = getnatp(victim)) == 0)
	return;
    if ((rel = getrel(np, actor)) < HOSTILE)
	return;

    rel = HOSTILE;
/*
	if (rel > HOSTILE)
		rel = HOSTILE;
	else
		rel = AT_WAR;
 */
    setrel(victim, actor, rel);
}

struct free {
    struct free *next;
    int id;
};

struct free *freelist;

static void
addfree(int n)
{
    struct free *fp;

    fp = (struct free *)malloc(sizeof(*fp));
    fp->next = freelist;
    fp->id = n;
    freelist = fp;
}

/*
 * snoop through the news articles looking
 * for articles which have timed out.  Only
 * called when no free items left.
 */
static void
findfree(void)
{
    register time_t oldnewstime;
    register int n;
    struct nwsstr news;
    time_t newstime;

    (void)time(&newstime);
    oldnewstime = newstime - NEWS_PERIOD;
    for (n = 0; getnews(n, &news); n++) {
	if (news.nws_when < oldnewstime)
	    addfree(n);
    }
    if (freelist == 0) {
	if (!ef_extend(EF_NEWS, 100))
	    return;
	findfree();
    }
}

static int
nextfree(void)
{
    struct free *fp;
    int id;

    if (freelist == 0)
	findfree();
    if ((fp = freelist) == 0)
	return 0;
    freelist = fp->next;
    id = fp->id;
    free(fp);
    return id;
}

#define SLOTS	5

struct newscache {
    struct nwsstr news;
    int id;
};

static struct newscache *
ncache(time_t now, int actor, int event, int victim, int times)
{
    static struct newscache cache[MAXNOC][SLOTS];
    register struct newscache *np;
    int i;
    int oldslot;
    time_t oldtime;

    oldslot = -1;
    oldtime = 0x7fffffff;
    for (i = 0; i < SLOTS; i++) {
	np = &cache[actor][i];
	if (np->news.nws_when < oldtime) {
	    oldslot = i;
	    oldtime = np->news.nws_when;
	}
	if (np->id == 0)
	    continue;
	if ((now - np->news.nws_when) > minutes(5))
	    continue;
	if (np->news.nws_vrb == event && np->news.nws_vno == victim &&
	    np->news.nws_ntm + times <= 127) {
	    np->news.nws_ntm += times;
	    return np;
	}
    }
    if (oldslot < 0) {
	logerror("internal error; ncache oldslot < 0");
	return &cache[actor][0];
    }
    np = &cache[actor][oldslot];
    np->news.nws_ano = actor;
    np->news.nws_vno = victim;
    np->news.nws_when = now;
    np->news.nws_vrb = event;
    np->news.nws_ntm = times;
    np->id = nextfree();
    return np;
}

static void
filereport(int actor, int event, int victim, int times)
{
    struct newscache *np;
    time_t now;

    time(&now);
    np = ncache(now, actor, event, victim, times);
    ef_write(EF_NEWS, np->id, (s_char *)&np->news);
}
