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
 *  nreport.c: File a news report.  Downgrade relations if things get hostile.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1994
 *     Steve McClure, 1997
 *     Ron Koenderink, 2005
 */

#include <config.h>

#include "file.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "prototypes.h"

#define SLOTS	5

static struct nwsstr cache[MAXNOC][SLOTS];
static int news_tail;

static struct nwsstr *ncache(int actor, int event, int victim, int times);

void
nreport(natid actor, int event, natid victim, int times)
{
    int nice;
    struct natstr *natp;
    struct nwsstr *np;

    if (CANT_HAPPEN((unsigned)event > N_MAX_VERB
		    || rpt[event].r_newstory[0] == rpt[0].r_newstory[0]))
	return;

    np = ncache(actor, event, victim, times);
    putnews(np->nws_uid, np);

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
    if (getrel(natp, actor) < HOSTILE)
	return;

    setrel(victim, actor, HOSTILE);
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
	news.nws_uid = j;
	putnews(j, &news);
    }
    CANT_HAPPEN(i + j != news_tail);
    news_tail = j;

    /* mark slots no longer in use */
    for (k = 0; k < i; k++) {
	ef_blank(EF_NEWS, j + k, &news);
	putnews(j + k, &news);
    }

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
static struct nwsstr *
ncache(int actor, int event, int victim, int times)
{
    struct nwsstr *np;
    int i;
    int oldslot;
    time_t oldtime;
    time_t now = time(NULL);

    oldslot = -1;
    oldtime = 0x7fffffff;
    for (i = 0; i < SLOTS; i++) {
	np = &cache[actor][i];
	if (np->nws_when < oldtime) {
	    oldslot = i;
	    oldtime = np->nws_when;
	}
	if (np->nws_uid == 0)
	    continue;
	if (now - np->nws_when > minutes(5))
	    continue;
	if (np->nws_vrb == event && np->nws_vno == victim
	    && np->nws_ntm + times <= 127) {
	    np->nws_ntm += times;
	    return np;
	}
    }
    if (CANT_HAPPEN(oldslot < 0))
	oldslot = 0;
    if (CANT_HAPPEN(!strstr(rpt[event].r_newstory[0], "%s") && victim != 0))
	victim = 0;
    np = &cache[actor][oldslot];
    ef_blank(EF_NEWS, news_tail, np);
    np->nws_ano = actor;
    np->nws_vno = victim;
    np->nws_when = now;
    np->nws_vrb = event;
    np->nws_ntm = times;
    news_tail++;
    return np;
}
