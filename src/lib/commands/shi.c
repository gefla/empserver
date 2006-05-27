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
 *  shi.c: Show a list of your ships
 * 
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "ship.h"
#include "nsc.h"
#include "file.h"
#include "optlist.h"
#include "commands.h"

int
shi(void)
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
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type       x,y   fl  eff civ mil  uw  fd pn");
	    pr(" he");
	    pr(" xl");
	    pr(" ln");
	    pr(" mob");
	    if (opt_FUEL)
		pr(" fuel");
	    pr(" tech\n");
	}
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d ", ni.cur);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y, player->cnum);
	pr("%1.1s", &ship.shp_fleet);
	pr("%4d%%", ship.shp_effic);

	pr("%4d", ship.shp_item[I_CIVIL]);
	pr("%4d", ship.shp_item[I_MILIT]);
	pr("%4d", ship.shp_item[I_UW]);
	pr("%4d", ship.shp_item[I_FOOD]);

	pr("%3d", ship.shp_nplane);
	pr("%3d", ship.shp_nchoppers);
	pr("%3d", ship.shp_nxlight);
	pr("%3d", ship.shp_nland);
	pr("%4d", ship.shp_mobil);
	if (opt_FUEL)
	    pr("%5d", ship.shp_fuel);
	pr("%5d\n", ship.shp_tech);
	if (ship.shp_name[0] != 0) {
	    if (player->god)
		pr("    ");
	    pr("       %s\n", ship.shp_name);
	}
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
