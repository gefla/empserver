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
 *  wantupd.c: Check to se if an update is wanted and/or allowed.
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1990
 */

#include <config.h>

#include <fcntl.h>
#include <stdio.h>
#if defined(_WIN32) && !defined(__GNUC__)
#include <io.h>
#endif
#include <time.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"
#include "wantupd.h"

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
    if (update_wantmin < 1)
	update_wantmin = 1;
    if (update_wantmin > MAXNOC)
	update_wantmin = MAXNOC;
    if (blitz_time < 1)
	blitz_time = 1;
}

static int
demand_update_time(time_t *now)
{
    struct tm *tm;

    tm = localtime(now);
    return is_daytime_allowed(60 * tm->tm_hour + tm->tm_min,
			      update_demandtimes);
}

/* When is the next regularly scheduled update from now. */
static void
regular_update_time(time_t *now, time_t *tim, time_t *delta)
{
    time_t tw;
    int secs_per_update;

    tw = *now + adj_update;
    secs_per_update = etu_per_update * s_p_etu;
    *delta = secs_per_update - (tw % secs_per_update);
    *tim = *now + *delta;
}

/* Is this a valid time for a scheduled update. */
static int
scheduled_update_time(time_t *now)
{
    struct tm *tm;

    tm = localtime(now);
    return is_daytime_near(60 * tm->tm_hour + tm->tm_min,
			   update_times, hourslop);
}

static int
next_scheduled_time(time_t *now, time_t *tim, time_t *delta)
{
    struct tm *tm;
    int d;

    tm = localtime(now);
    d = min_to_next_daytime(60 * tm->tm_hour + tm->tm_min, update_times);
    if (d < 0)
	return 0;
    *delta = 60 * d;
    *tim = *now + *delta - tm->tm_sec;
    return 1;
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
	if (natp->nat_stat == STAT_ACTIVE) {
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
    return whichwants;
}

static int
demand_check(void)
{
    struct natstr *natp;
    int want, pop, cn, veto;
    time_t now;
    time_t cur;

    time(&cur);

    demand_update_want(&want, &pop, 0);
    if (want < update_wantmin) {
	logerror("no demand update, want = %d, min = %d",
		 want, update_wantmin);
	return 0;
    }

    time(&now);
    if (!demand_update_time(&now)) {
	logerror("no demand update, not within hours allowed.");
	return 0;
    }


    veto = 0;
    for (cn = 1; 0 != (natp = getnatp(cn)); cn++) {
	if (natp->nat_stat == STAT_ACTIVE) {
	    if (natp->nat_missed >= update_missed)
		veto = cn;
	}
    }

    if (veto) {
	logerror("no demand update, %d has missed more than %d updates",
		 veto, update_missed);
	return 0;
    }

    return 1;
}

/*
 * Check if enough countries want an update,
 * and if demand updates are allowed now.
 */
int
demandupdatecheck(void)
{
    if (UDDEM_COMSET != update_demandpolicy) {
	logerror("no demand update, not policy.");
	return 0;
    }

    return demand_check();
}

/*
 * Is it time for a regular or scheduled update?
 * As well, if none of the above, check to see if
 * a demand update can occur.
 */
int
updatetime(time_t *now)
{
    if (opt_BLITZ && update_policy == UDP_BLITZ) {
	logerror("BLITZ Update.");
	return 1;
    }

    if (UDP_NORMAL == update_policy) {
	logerror("Regular update, etu type.");
	return 1;
    }

    if (UDP_TIMES == update_policy) {
	if (scheduled_update_time(now)) {
	    logerror("Scheduled update.");
	    return 1;
	}
    }
    if (opt_DEMANDUPDATE) {
	if (demand_check()) {
	    logerror("Demand update, at check time.");
	    return 1;
	}
    }
    return 0;
}

/*
 * Return the time, and delta seconds, of the next update.
 * If the policy is no regular updates, return the time of
 * the next possible check.
 */
void
next_update_time(time_t *now, time_t *tim, time_t *delta)
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
next_update_check_time(time_t *now, time_t *tim, time_t *delta)
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
    int fd;

    if ((fd = open(disablefil, O_RDONLY, 0)) < 0)
	return 0;
    close(fd);
    return 1;
}
