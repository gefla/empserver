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
 *  shi.c: Show a list of your ships
 *
 *  Known contributors to this file:
 *     Steve McClure, 2000
 */

#include <config.h>

#include "commands.h"
#include "ship.h"

int
shi(void)
{
    int nships, noff, npln, nch, nxl;
    struct nstr_item ni;
    struct shpstr ship;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;

    nships = noff = 0;
    while (nxtitem(&ni, &ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	if (nships++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type       x,y   fl   eff civ mil  uw  fd pn"
	       " he xl ln mob tech\n");
	}
	if (ship.shp_off)
	    noff++;
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d ", ni.cur);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y);
	pr("%1.1s", &ship.shp_fleet);
	pr(" %c%3d%%", ship.shp_off ? '!' : ' ', ship.shp_effic);

	pr("%4d", ship.shp_item[I_CIVIL]);
	pr("%4d", ship.shp_item[I_MILIT]);
	pr("%4d", ship.shp_item[I_UW]);
	pr("%4d", ship.shp_item[I_FOOD]);

	npln = shp_nplane(&ship, &nch, &nxl, NULL);
	pr("%3d%3d%3d", npln - nch - nxl, nch, nxl);
	pr("%3d", shp_nland(&ship));
	pr("%4d", ship.shp_mobil);
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
    } else {
	pr("%d ship%s", nships, splur(nships));
	if (noff)
	    pr(", %d stopped (eff marked with !)", noff);
	pr("\n");
    }
    return RET_OK;
}
