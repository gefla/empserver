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
 *  upda.c: Give the time of the next update
 * 
 *  Known contributors to this file:
 *  
 */

#include <stdio.h>
#include <sys/types.h>
#include "misc.h"
#include "player.h"
#include "commands.h"
#include "optlist.h"
#include "wantupd.h"
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
    if (opt_UPDATESCHED) {
	time_t now, next, delta;

	if (updates_disabled())
	    pr("UPDATES ARE DISABLED!\n");

	(void)time(&now);
	switch (update_policy) {
	case UDP_NORMAL:
	    next_update_time(&now, &next, &delta);
	    pr("\nUpdates occur at times specified by the ETU rates.\n\n");
	    pr("The next update is at %19.19s.\n", ctime(&next));
	    break;
	case UDP_TIMES:
	    next_update_time(&now, &next, &delta);
	    pr("\nUpdates occur at scheduled times.\n\n");
	    pr("The next update is at %19.19s.\n", ctime(&next));
	    break;
	case UDP_BLITZ:
	    next_update_time(&now, &next, &delta);
	    pr("\nBlitz Updates occur every %d minutes. \n\n", blitz_time);
	    pr("The next update is at %19.19s.\n", ctime(&next));
	    break;
	case UDP_NOREG:
	    pr("There are no regularly scheduled updates.\n");
	    break;
	default:
	    pr("Update policy is inconsistent.\n");
	}
	pr("The current time is   %19.19s.\n\n", ctime(&now));

	if (update_window) {
	    now = update_time - update_window;
	    next_update_time(&now, &next, &delta);
	    pr("The next update window starts at %19.19s.\n",
	       ctime(&next));
	    next += update_window;
	    pr("The next update window stops at %19.19s.\n", ctime(&next));
	}
	if (opt_DEMANDUPDATE) {
	    if (update_demandpolicy != UDDEM_DISABLE) {
		switch (update_demandpolicy) {
		case UDDEM_TMCHECK:
		    next_update_check_time(&now, &next, &delta);
		    pr("Demand updates occur at update CHECK times.\n");
		    pr("The next update check is at %19.19s.\n",
		       ctime(&next));
		    break;
		case UDDEM_COMSET:
		    pr("Demand updates occur right after the demand is set.\n");
		    break;
		default:
		    pr("Update demand policy is inconsistent.\n");
		}
	    }
	}

	if ((update_policy == UDP_TIMES) ||
	    ((update_demandpolicy == UDDEM_TMCHECK) && opt_DEMANDUPDATE)) {
	    if (*update_times != 0)
		pr("The update schedule is: %s\n", update_times);
	}
	if (opt_DEMANDUPDATE) {
	    if (update_demandpolicy != UDDEM_DISABLE) {
		if (*update_demandtimes != 0)
		    pr("Demand updates are allowed during: %s\n",
		       update_demandtimes);
		if (update_wantmin == 0) {
		    pr("Demand updates are disabled by a mininum of 0\n");
		} else {
		    pr("Demand updates require %d country(s) to want one.\n", update_wantmin);
		}
	    }
	}
	if (*game_days != 0)
	    pr("Game days are: %s\n", game_days);
	if (*game_hours != 0)
	    pr("Game hours are: %s\n", game_hours);

	return (0);
    } else {
	time_t now;
	time_t upd_time;
	time_t next_update;
	int secs_per_update;
	int delta;

	(void)time(&now);
	upd_time = now + adj_update;
	secs_per_update = etu_per_update * s_p_etu;
	delta = secs_per_update - (upd_time % secs_per_update);
	next_update = now + delta;
	pr("The next update is at %19.19s.\n", ctime(&next_update));
	pr("The current time is %19.19s.\n", ctime(&now));
	if (update_window) {
	    pr("Update times are variable, update window is +/- %d minutes %d seconds.\n", update_window / 60, update_window % 60);
	}
	return 0;
    }
}
