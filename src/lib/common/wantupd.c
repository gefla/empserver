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
 *  wantupd.c: Check to se if an update is wanted and/or allowed.
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1990
 */

#include <stdio.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "misc.h"
#include "nat.h"
#include "file.h"
#include "keyword.h"
#include "wantupd.h"
#include "optlist.h"
#include "common.h"

#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

void
update_policy_check(void)
{
    if (update_policy < 0)
	update_policy = UDP_DEFAULT;
    if (update_policy > UDP_MAX)
	update_policy = UDP_DEFAULT;
    if (update_demandpolicy < 0)
	update_demandpolicy = UDDEM_DEFAULT;
    if (update_demandpolicy > UDDEM_MAX)
	update_demandpolicy = UDDEM_DEFAULT;
    if (update_wantmin < 0)
	update_wantmin = 0;
    if (update_wantmin > MAXNOC)
	update_wantmin = MAXNOC;
    if (blitz_time < 0)
	blitz_time = 0;
}

static int
demand_update_time(time_t * now)
{
    struct tm *tm;
    s_char *p;
    int curtime;
    int hour[2];

    tm = localtime(now);
    curtime = tm->tm_min + tm->tm_hour * 60;
    p = update_demandtimes;
    if (*p == 0)
	return (1);
    while (NULL != (p = kw_parse(CF_TIMERANGE, p, &hour[0]))) {
	if (curtime >= hour[0] && curtime < hour[1])
	    return (1);
    }
    return (0);
}

/* When is the next regularly scheduled update from now. */
static void
regular_update_time(time_t * now, time_t * tim, time_t * delta)
{
    s_char *p;
    time_t tw;
    int secs_per_update;

    tw = *now + adj_update;
    secs_per_update = etu_per_update * s_p_etu;
    *delta = secs_per_update - (tw % secs_per_update);
    *tim = *now + *delta;
}

/* Is this a valid time for a scheduled update. */
static int
scheduled_update_time(time_t * now, int *which)
{
    struct tm *tm;
    s_char *p, *p1;
    int curtime;
    int hour;

    *which = -1;
    p = update_times;
    if (*p == 0)
	return (0);

    tm = localtime(now);
    curtime = tm->tm_min + tm->tm_hour * 60;
    while (NULL != (p = kw_parse(CF_TIME, p, &hour))) {
	(*which)++;
	if (curtime >= hour && curtime < hour + hourslop)
	    return (1);
    }

    return 0;
}

static int
next_scheduled_time(time_t * now, time_t * tim, time_t * delta)
{
    struct tm *tm;
    s_char *p;
    int curtime;
    int hour;
    int mintime;

    p = update_times;
    if (*p == 0)
	return (0);

    tm = localtime(now);
    curtime = tm->tm_min + tm->tm_hour * 60;	/* now - in minutes */
    mintime = curtime + 24 * 60 + 1;	/* start with now + 1 day */
    while (NULL != (p = kw_parse(CF_TIME, p, &hour))) {
	if (hour <= curtime)
	    hour += 24 * 60;	/* this must be tomorrow */
	if (hour < mintime)
	    mintime = hour;	/* this is best bet so far */
    }
    *delta = 60 * (mintime - curtime);
    *tim = *now + *delta - tm->tm_sec;
    return (1);
}

int
demand_update_want(int *want, int *pop, int which)
{
    natid cn;
    struct natstr *natp;
    int totpop;
    int totwant;
    int whichwants;

    whichwants = totpop = totwant = 0;
    for (cn = 1; 0 != (natp = getnatp(cn)); cn++) {
	/* Only countries which are normal. */
	/* Should probably include sanctuaries ..... */
	if (((natp->nat_stat & NORM) == NORM) &&
	    ((natp->nat_stat & GOD) != GOD)) {
	    totpop++;
	    if ((natp->nat_update & WUPD_WANT) == WUPD_WANT) {
		totwant++;
		if (which == cn)
		    whichwants++;
	    }
	}
    }
    *want = totwant;
    *pop = totpop;
    return (whichwants);
}

static int
demand_check(void)
{
    struct natstr *natp;
    int want, pop, cn, veto;
    time_t now;
    time_t cur;

    time(&cur);

/*
	if (last_demand_update == 0){
		natp=getnatp(0);
		last_demand_update = natp->nat_reserve;
	}

	logerror("last_demand_update = %d\n",last_demand_update);
	logerror("update_between = %d\n",update_between());
	logerror("now = %d\n",cur);
	diff = (cur-(last_demand_update + update_between()));
	logerror("diff = %d\n",diff);
	if (diff >= 0){
		logerror("Forced update!\n");
		last_demand_update = cur;
		for (cn = 1; natp = getnatp(cn); cn++){
			if (((natp->nat_stat & NORM) == NORM)  &&
				((natp->nat_stat & GOD) != GOD)){
				natp->nat_missed = 0;
			}
		}
		return(1);
	}

	logerror("No forced update!\n");
*/
    if (0 == update_wantmin) {
	logerror("no demand update allowed, wantmin = 0");
	return (0);
    }

    demand_update_want(&want, &pop, 0);
    if (want < update_wantmin) {
	logerror("no demand update, want = %d, min = %d",
		 want, update_wantmin);
	return (0);
    }

    time(&now);
    if (!demand_update_time(&now)) {
	logerror("no demand update, not within hours allowed.");
	return (0);
    }


    veto = 0;
    for (cn = 1; 0 != (natp = getnatp(cn)); cn++) {
	if (((natp->nat_stat & NORM) == NORM) &&
	    ((natp->nat_stat & GOD) != GOD)) {
	    if (natp->nat_missed >= update_missed)
		veto = cn + 1;
	}
    }

    if (veto) {
	logerror("no demand update, %d has missed more than %d updates",
		 veto - 1, update_missed);
	return (0);
    }

    last_demand_update = cur;
    natp = getnatp(0);
    /* A dumb way to do it, but simple */
    last_demand_update = natp->nat_reserve;
    return (1);
}

/* Check if enough countries want an update,
 * and if demand updates are allowed now.
 */
int
demandupdatecheck(void)
{
    if (UDDEM_COMSET != update_demandpolicy) {
	logerror("no demand update, not policy.");
	return (0);
    }

    return (demand_check());
}

/* Is it time for a regular or scheduled update?
 * As well, if none of the above, check to see if
 * a demand update can occur.
 */
int
updatetime(time_t * now)
{
    int which;

    if (opt_BLITZ && update_policy == UDP_BLITZ) {
	logerror("BLITZ Update.");
	return (1);
    }

    if (UDP_NORMAL == update_policy) {
	logerror("Regular update, etu type.");
	return (1);
    }

    if (UDP_TIMES == update_policy) {
	if (scheduled_update_time(now, &which)) {
	    logerror("Scheduled update, %d.", which);
	    return (1);
	}
    }
    if (opt_DEMANDUPDATE) {
	if (demand_check()) {
	    logerror("Demand update, at check time.");
	    return (1);
	}
    }
    return (0);
}

/* Return the time, and delta seconds, of the next update.
 * If the policy is no regular updates, return the time of
 * the next possible check.
 */
void
next_update_time(time_t * now, time_t * tim, time_t * delta)
			/* From when */
			/* Time of next update */
			/* Seconds till next update */
{
    time_t stim, sdelta;

    switch (update_policy) {
    case UDP_NORMAL:
	regular_update_time(now, tim, delta);
	break;
    case UDP_TIMES:
	if (!next_scheduled_time(now, tim, delta))
	    regular_update_time(now, tim, delta);
	break;
    case UDP_BLITZ:
	*delta = (blitz_time * 60) - (*now % (blitz_time * 60));
	*tim = *now + *delta;
	break;

    case UDP_NOREG:
    default:
	regular_update_time(now, tim, delta);
	if (next_scheduled_time(now, &stim, &sdelta)) {
	    if (*delta > sdelta) {
		*delta = sdelta;
		*tim = stim;
	    }
	}
	break;
    }
}

void
next_update_check_time(time_t * now, time_t * tim, time_t * delta)
			/* From when */
			/* Time of next update */
			/* Seconds till next update check */
{
    time_t stim, sdelta;

    switch (update_policy) {
    case UDP_NORMAL:
	regular_update_time(now, tim, delta);
	break;
    case UDP_BLITZ:
	*delta = (blitz_time * 60) - (*now % (blitz_time * 60));
	*tim = *now + *delta;
	break;
    case UDP_TIMES:
    case UDP_NOREG:
    default:
	regular_update_time(now, tim, delta);
	if (next_scheduled_time(now, &stim, &sdelta)) {
	    if (*delta > sdelta) {
		*delta = sdelta;
		*tim = stim;
	    }
	}
    }
}

int
updates_disabled(void)
{
    extern s_char *disablefil;
    int fd;

    if ((fd = open(disablefil, O_RDONLY, 0)) < 0)
	return 0;
    close(fd);
    return 1;
}
