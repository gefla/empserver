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
 *  sabo.c: Spy terrorist bombing
 *
 *  Known contributors to this file:
 *     John Yockey, 2001
 */

#include <config.h>

#include "commands.h"
#include "land.h"

int
sabo(void)
{
    struct nstr_item ni;
    struct lndstr land;
    struct sctstr sect;
    double odds;
    int dam;

    if (!snxtitem(&ni, EF_LAND, player->argp[1], NULL))
	return RET_SYN;

    while (nxtitem(&ni, &land)) {
	if (!player->owner)
	    continue;
	if (!(lchr[(int)land.lnd_type].l_flags & L_SPY)) {
	    pr("%s is not a spy.\n", prland(&land));
	    continue;
	}
	if (land.lnd_ship >= 0) {
	    pr("%s is on ship %d.\n", prland(&land), land.lnd_ship);
	    continue;
	}
	if (land.lnd_land >= 0) {
	    pr("%s is on unit %d.\n", prland(&land), land.lnd_land);
	    continue;
	}
	if (!getsect(land.lnd_x, land.lnd_y, &sect))
	    continue;
	if (land.lnd_item[I_SHELL] == 0) {
	    pr("%s has no shells.\n", prland(&land));
	    continue;
	}
	--land.lnd_item[I_SHELL];

	odds = LND_SPY_DETECT_CHANCE(land.lnd_effic);
	if (chance(odds)) {
	    wu(0, sect.sct_own,
	       "%s spy shot in %s during sabotage attempt.\n",
	       cname(player->cnum),
	       xyas(sect.sct_x, sect.sct_y, sect.sct_own));
	    pr("%s was shot and killed.\n", prland(&land));
	    land.lnd_effic = 0;
	    putland(land.lnd_uid, &land);
	    continue;
	}

	dam = fortgun(3 * land.lnd_effic, 7);
	if (sect.sct_item[I_SHELL] > 20)
	    dam += seagun(land.lnd_effic,
			  random() % (sect.sct_item[I_SHELL] / 10));
	if (sect.sct_item[I_PETROL] > 100)
	    dam += seagun(land.lnd_effic,
			  random() % (sect.sct_item[I_PETROL] / 50));

	pr("Explosion in %s causes %d damage.\n",
	   xyas(land.lnd_x, land.lnd_y, land.lnd_own), dam);
	if (sect.sct_own) {
	    wu(0, sect.sct_own,
	       "Sabotage in sector %s caused %d damage.\n",
	       xyas(sect.sct_x, sect.sct_y, sect.sct_own), dam);
	}

	/* hack: hide the spy so it don't gets blasted by sectdamage() */
	land.lnd_own = 0;
	putland(land.lnd_uid, &land);

	sectdamage(&sect, dam);
	putsect(&sect);

	land.lnd_own = player->cnum;
	if (chance(odds)) {
	    pr("%s dies in explosion.\n", prland(&land));
	    land.lnd_effic = 0;
	}
	putland(land.lnd_uid, &land);
    }
    return RET_OK;
}
