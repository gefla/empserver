/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  payo.c: Calculate trade ship payoffs
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1992
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "ship.h"

int
payo(void)
{
    struct sctstr sect;
    int nships;
    struct nstr_item ni;
    struct shpstr ship;
    struct mchrstr *mp;
    int dist;
    float cash = 0.0;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;

    nships = 0;
    while (nxtitem(&ni, &ship)) {
	if (!player->owner || ship.shp_own == 0)
	    continue;
	mp = &mchr[(int)ship.shp_type];

	if (!(mp->m_flags & M_TRADE))
	    continue;

	if (nships++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("shp#     ship type  orig x,y       x,y    dist $$\n");
	}
	if (player->god)
	    pr("%3d ", ship.shp_own);
	pr("%4d ", ni.cur);
	pr("%-16.16s ", mchr[(int)ship.shp_type].m_name);
	if (ship.shp_own != ship.shp_orig_own && !player->god) {
	    /* Don't disclose construction site to pirates! */
	    pr("    ?     ");
	    prxy("%4d,%-4d ", ship.shp_x, ship.shp_y);
	    pr("   ? $  ?\n");
	    continue;
	}
	prxy("%4d,%-4d ", ship.shp_orig_x, ship.shp_orig_y);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y);

	getsect(ship.shp_x, ship.shp_y, &sect);

	dist = mapdist(ship.shp_x, ship.shp_y,
		       ship.shp_orig_x, ship.shp_orig_y);
	pr("%4d ", dist);

	if (dist < trade_1_dist)
	    cash = 0;
	else if (dist < trade_2_dist)
	    cash = 1.0 + trade_1 * dist;
	else if (dist < trade_3_dist)
	    cash = 1.0 + trade_2 * dist;
	else
	    cash = 1.0 + trade_3 * dist;

	cash *= mp->m_cost;
	cash *= ship.shp_effic / 100.0;

	if (sect.sct_own && (sect.sct_own != ship.shp_own))
	    cash *= (1.0 + trade_ally_bonus);
	pr("$%6.2f\n", cash);
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
