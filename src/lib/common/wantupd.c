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
 *  wantupd.c: Check to se if an update is wanted and/or allowed.
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1990
 */

#include <config.h>

#include <time.h>
#include "file.h"
#include "game.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"

static int
demand_update_time(time_t *now)
{
    struct tm *tm;

    tm = localtime(now);
    return is_daytime_allowed(60 * tm->tm_hour + tm->tm_min,
			      update_demandtimes);
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
	    if (natp->nat_update) {
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

/*
 * Do we have sufficient votes for a demand update?
 */
int
demand_check(void)
{
    int want, pop;

    demand_update_want(&want, &pop, 0);
    if (want < update_wantmin) {
	logerror("no demand update, want = %d, min = %d",
		 want, update_wantmin);
	return 0;
    }

    return 1;
}

/*
 * Can we have an unscheduled demand update now?
 */
int
demandupdatecheck(void)
{
    time_t now = time(NULL);

    return update_demand == UPD_DEMAND_ASYNC
	&& !updates_disabled()
	&& demand_update_time(&now)
	&& demand_check();
}
