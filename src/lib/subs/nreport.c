/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

#include "prototypes.h"
#include "news.h"
#include "file.h"
#include "optlist.h"

#define SLOTS	5

struct newscache {
    struct nwsstr news;
    int id;
};

static struct newscache cache[MAXNOC][SLOTS];
static int news_tail;

static struct newscache *
ncache(int actor, int event, int victim, int times);

void
nreport(natid actor, int event, natid victim, int times)
{
    int nice;
    int rel;
    struct natstr *natp;
    struct newscache *ncp;

    ncp = ncache(actor, event, victim, times);
    putnews(ncp->id, &ncp->news);

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
    if ((natp = getnatp(victim)) == 0)
	return;
    if ((rel = getrel(natp, actor)) < HOSTILE)
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

/*
 * Delete news articles that have expired.
 */
void
delete_old_news(void)
{
    time_t expiry_time;
    int i, j, k;
    struct nwsstr news;

    /* skip over expired news */
    expiry_time = time(NULL) - days(news_keep_days);
    for (i = 0; getnews(i, &news); i++) {
	if (news.nws_when == 0 || news.nws_when >= expiry_time)
	    break;
    }
    /* news id 0..I-1 have expired */
    CANT_HAPPEN(i > news_tail);
    /* no items to delete if I is equal zero */
    if (i == 0)
	return;

    /* move unexpired news I.. to 0.., overwriting expired news */
    for (j = 0; getnews(i + j, &news); j++) {
	if (news.nws_when == 0)
	    break;
	putnews(j, &news);
    }
    CANT_HAPPEN(i + j != news_tail);
    news_tail = j;

    /* mark slots no longer in use */
    memset(&news, 0, sizeof(news));
    for (k = 0; k < i; k++)
	putnews(j + k, &news);

    /* clear cache because moving news invalidated it */
    memset(&cache, 0, sizeof(cache));
}

/*
 * Initialize news reporting.
 * Must run between open of file EF_NEWS and first nreport().
 */
void
init_nreport(void)
{
    int newest_item;
    struct nwsstr news;

    for (newest_item = 0; getnews(newest_item, &news); newest_item++) {
	if (news.nws_when == 0)
	    break;
    }
    news_tail = newest_item;
}

/*
 * Look to see if the same message has been generated
 * in the last 5 minutes, if so just increment the times
 * field instead of creating a new message.
 */
static struct newscache *
ncache(int actor, int event, int victim, int times)
{
    register struct newscache *np;
    int i;
    int oldslot;
    time_t oldtime;
    time_t now = time(NULL);

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
    ef_ensure_space(EF_NEWS, news_tail, 100);
    np->id = news_tail++;
    return np;
}
