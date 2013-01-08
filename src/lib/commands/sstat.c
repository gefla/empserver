/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sstat.c: Show the stats of your ships
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"
#include "ship.h"

int
sstat(void)
{
    int nships;
    struct nstr_item ni;
    struct shpstr ship;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;

    nships = 0;
    while (nxtitem(&ni, &ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (nships++ == 0) {
	    pr("shp# %22.22s    x,y    eff  tech def spd vis rng fir\n",
	       "ship-type");
	}
	pr("%4d %-22.22s ", ship.shp_uid, mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d", ship.shp_x, ship.shp_y);
	pr(" %3d%% %4d %3d %3d %3d %3d %3d",
	   ship.shp_effic,
	   ship.shp_tech,
	   shp_armor(&ship), shp_speed(&ship), shp_visib(&ship),
	   shp_frnge(&ship), shp_glim(&ship));
	pr("\n");
    }
    if (nships == 0) {
	if (player->argp[1])
	    pr("%s: No ship(s)\n", player->argp[1]);
	else
	    pr("%s: No ship(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d ship%s\n", nships, splur(nships));
    return RET_OK;
}
