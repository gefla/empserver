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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  sstat.c: Show the stats of your ships
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "ship.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

int
sstat(void)
{
    int nships;
    struct nstr_item ni;
    struct shpstr ship;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1]))
	return RET_SYN;

    nships = 0;
    while (nxtitem(&ni, &ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	count_planes(&ship);
	count_units(&ship);

	if (nships++ == 0) {
	    pr("shp# %22.22s    x,y    eff  tech def spd vis rng fir\n",
	       "ship-type");
	}
	pr("%4d %-22.22s ", ship.shp_uid, mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d", ship.shp_x, ship.shp_y, player->cnum);
	pr(" %3d%% %4d %3d %3d %3d %3d %3d",
	   ship.shp_effic,
	   ship.shp_tech,
	   ship.shp_armor,
	   ship.shp_speed, ship.shp_visib, ship.shp_frnge, ship.shp_glim);
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
