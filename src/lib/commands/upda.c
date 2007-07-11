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
 *  upda.c: Give the time of the next update
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "server.h"

/*
 * Tell what the update policy is, and when the next update
 * is likely to be.
 */
int
upda(void)
{
    FILE *fp;
    struct mob_acc_globals timestamps;
    time_t now, next, stop;

    if (opt_MOB_ACCESS) {
	if ((fp = fopen(timestampfil, "rb")) == NULL)
	    logerror("Unable to open timestamp file.");
	else {
	    rewind(fp);
	    fread(&timestamps, sizeof(timestamps), 1, fp);
	    fclose(fp);
	    if (updating_mob)
		pr("Mobility updating is enabled.\n\n");
	    else {
		pr("Mobility updating will come back on around %s",
		   ctime(&timestamps.starttime));
		pr("game time, within 3 minutes, depending on when the server checks.\n\n");
	    }
	}
    }

    if (updates_disabled())
	pr("UPDATES ARE DISABLED!\n");

    (void)time(&now);
    next = update_time[0];
    if (next) {
	pr("\nUpdates occur at times specified by the ETU rates.\n\n");
	pr("The next update is at %19.19s.\n", ctime(&next));
    } else {
	pr("There are no regularly scheduled updates.\n");
    }
    pr("The current time is   %19.19s.\n\n", ctime(&now));

    if (next && update_window) {
	pr("The next update window starts at %19.19s.\n",
	   ctime(&next));
	stop = next + update_window;
	pr("The next update window stops at %19.19s.\n", ctime(&stop));
    }

    switch (update_demand) {
    case UPD_DEMAND_NONE:
    default:
	break;
    case UPD_DEMAND_SCHED:
	pr("Demand updates occur at update CHECK times.\n");
	if (next) {
	    pr("The next update check is at %19.19s.\n",
	       ctime(&next));
	}
	pr("Demand updates require %d country(s) to want one.\n",
	   update_wantmin);
	break;
    case UPD_DEMAND_ASYNC:
	pr("Demand updates occur right after the demand is set.\n");
	if (*update_demandtimes != 0) {
	    pr("Demand updates are allowed during: %s\n",
	       update_demandtimes);
	}
	pr("Demand updates require %d country(s) to want one.\n",
	   update_wantmin);
    }

    if (*game_days != 0)
	pr("Game days are: %s\n", game_days);
    if (*game_hours != 0)
	pr("Game hours are: %s\n", game_hours);

    return 0;
}
