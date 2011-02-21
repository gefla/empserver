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
 *  zdon.c: Update right now
 *
 *  Known contributors to this file:
 *     Doug Hay, 1990
 *     Markus Armbruster, 2007-2008
 */

/*
 * Syntax:
 *  Normal player:   zdone [Y|N|C]
 *  Deity        :   zdone [country_num [Y|N|C]]
 *
 *  Where:
 *         Y  = Yes, wants an update.
 *         N  = No, change status to not wanting an update.
 *         C  = Check (the default), check how many want an update.
 *
 * Sets/Unsets a nation flag.
 *
 * Only considers NORMAL, active countries.  No Deities or sanctuaries.
 *
 * After the change, send a message to the "tm" for it to check
 * if an update should occur.
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "server.h"

int
zdon(void)
{
    int whichcnum;
    struct natstr *natp;
    char *p;

    int checking;
    int wantupd;
    int totpop;
    int totwant;
    int dowant;
    char buf[1024];

    if (update_demand != UPD_DEMAND_SCHED
	&& update_demand != UPD_DEMAND_ASYNC) {
	pr("Demand updates are not enabled.\n");
	return RET_FAIL;
    }
    p = getstarg(player->argp[1], "Want update? [Yes|No|Check] ", buf);
    if (!p)
	return RET_SYN;
    if (*p == 'y' || *p == 'Y') {
	checking = 0;
	wantupd = 1;
    } else if (*p == 'n' || *p == 'N') {
	checking = 0;
	wantupd = 0;
    } else {
	checking = 1;
	wantupd = 0;
    }

    if (player->god) {
	whichcnum = natarg(player->argp[2], "for which country? ");
	if (whichcnum < 0)
	    return RET_SYN;
    } else
	whichcnum = player->cnum;

    if (!(natp = getnatp(whichcnum))) {
	pr("Unable to find country. %d\n", whichcnum);
	pr("Notify the Deity.\n");
	return RET_FAIL;
    }
    if (!checking) {
	if (wantupd) {
	    if (influx(natp)) {
		pr("Unable to request an update as the country is in flux\n");
		return RET_FAIL;
	    }
	    pr("You (%d) now want an update.\n", whichcnum);
	} else {
	    pr("You (%d) now DON'T want an update.\n", whichcnum);
	}
	natp->nat_update = wantupd;
	putnat(natp);
    }

    dowant = demand_update_want(&totwant, &totpop, whichcnum);
    if (checking) {
	if (dowant) {
	    pr("You want an update.\n");
	} else
	    pr("You DON'T want an update, yet.\n");
    }

    pr("%d of a total of %d lunatics want an update.\n", totwant, totpop);

    if (!checking && wantupd && demandupdatecheck()) {
	pr("Here goes...\n");
	update_trigger();
    }
    return RET_OK;
}
