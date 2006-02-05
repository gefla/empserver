/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  zdon.c: Update right now
 * 
 *  Known contributors to this file:
 *     Doug Hay, 1990
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

#include <stdio.h>
#include "misc.h"
#include "player.h"
#include "nat.h"
#include "file.h"
#include "empthread.h"
#include "commands.h"
#include "optlist.h"
#include "server.h"

int
zdon(void)
{
    natid whichcnum;
    struct natstr *natp;
    register s_char *p;

    int update;
    int checking;
    int wantupd;
    int totpop;
    int totwant;
    int dowant;
    s_char buf[1024];

    if (!opt_DEMANDUPDATE) {
	pr("Demand updates are not enabled.\n");
	return RET_FAIL;
    }
    whichcnum = player->cnum;
    p = NULL;
    if (player->god) {
	/* Deity syntax "country what" */
	whichcnum = onearg(player->argp[1], "Which country no.? ");
	if ((whichcnum > 0) && (getnatp(whichcnum)))
	    p = getstarg(player->argp[2], "Want update? [Yes|No|Check] ",
			 buf);
    } else {
	p = getstarg(player->argp[1], "Want update? [Yes|No|Check] ", buf);
    }
    if (player->aborted)
	return RET_FAIL;

    if (!p) {
	/* Default response is checking only */
	checking = 1;
    } else {
	checking = 0;
	if (*p == 'n' || *p == 'N') {
	    wantupd = 0;
	} else if (*p == 'y' || *p == 'Y') {
	    wantupd = 1;
	} else {
	    /* Default response is checking only */
	    checking = 1;
	}
    }

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
	    update = natp->nat_update | WUPD_WANT;
	    natp->nat_missed = 0;
	    pr("You (%d) now want an update.\n", whichcnum);
	} else {
	    update = natp->nat_update & ~WUPD_WANT;
	    pr("You (%d) now DON'T want an update.\n", whichcnum);
	}
	natp->nat_update = update;
	putnat(natp);
    }

    dowant = demand_update_want(&totwant, &totpop, whichcnum);
    if (checking) {
	if (dowant) {
	    pr("You want an update.\n");
	} else
	    pr("You DON'T want an update, yet.\n");
	pr("You have missed requesting an demand update %d times.\n",
	   natp->nat_missed);
	pr("It takes %d misses to veto an demand update.\n",
	   update_missed);
    }

    pr("%d of a total of %d lunatics want an update.\n", totwant, totpop);

    if (!checking && wantupd && demandupdatecheck()) {
	pr("Here goes...\n");
	empth_sem_signal(update_sem);
    }
    return RET_OK;
}
