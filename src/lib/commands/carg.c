/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  carg.c: List cargo contents of a ship
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "commands.h"
#include "land.h"
#include "ship.h"

int
c_cargo(void)
{
    int nships;
    struct nstr_item ni;
    struct shpstr ship;

    if (!snxtitem(&ni, EF_SHIP, player->argp[1], NULL))
	return RET_SYN;
    nships = 0;
    while (nxtitem(&ni, &ship)) {
	if (ship.shp_own == 0)
	    continue;
	if ((player->cnum != ship.shp_own) && !player->god)
	    continue;
	if (nships++ == 0)
	    pr("shp#         x,y   flt eff  civ mil  uw  sh gun pet irn dst bar oil lcm hcm rad\n");
	pr("%4d ", ni.cur);
	pr("%-4.4s ", mchr[(int)ship.shp_type].m_name);
	prxy("%4d,%-4d ", ship.shp_x, ship.shp_y);
	pr("%1.1s", &ship.shp_fleet);
	pr("%4d%%", ship.shp_effic);
	pr("%4d", ship.shp_item[I_CIVIL]);
	pr("%4d", ship.shp_item[I_MILIT]);
	pr("%4d", ship.shp_item[I_UW]);
	pr("%4d", ship.shp_item[I_SHELL]);
	pr("%4d", ship.shp_item[I_GUN]);
	pr("%4d", ship.shp_item[I_PETROL]);
	pr("%4d", ship.shp_item[I_IRON]);
	pr("%4d", ship.shp_item[I_DUST]);
	pr("%4d", ship.shp_item[I_BAR]);
	pr("%4d", ship.shp_item[I_OIL]);
	pr("%4d", ship.shp_item[I_LCM]);
	pr("%4d", ship.shp_item[I_HCM]);
	pr("%4d\n", ship.shp_item[I_RAD]);
	if (ship.shp_name[0] != 0)
	    pr("       %s\n", ship.shp_name);
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

int
c_lcargo(void)
{
    int nunits;
    struct nstr_item ni;
    struct lndstr land;

    if (!snxtitem(&ni, EF_LAND, player->argp[1], NULL))
	return RET_SYN;
    nunits = 0;
    while (nxtitem(&ni, &land)) {
	if (!land.lnd_own)
	    continue;
	if ((player->cnum != land.lnd_own) && !player->god)
	    continue;
	if (nunits++ == 0)
	    pr("lnd# unit    x,y    a  eff  civ mil  uw  sh gun pet irn dst bar oil lcm hcm rad\n");
	pr("%4d ", ni.cur);
	pr("%-4.4s ", lchr[(int)land.lnd_type].l_name);
	prxy("%4d,%-4d ", land.lnd_x, land.lnd_y);
	pr("%1.1s ", &land.lnd_army);
	pr("%4d%%", land.lnd_effic);
	pr("%4d", land.lnd_item[I_CIVIL]);
	pr("%4d", land.lnd_item[I_MILIT]);
	pr("%4d", land.lnd_item[I_UW]);
	pr("%4d", land.lnd_item[I_SHELL]);
	pr("%4d", land.lnd_item[I_GUN]);
	pr("%4d", land.lnd_item[I_PETROL]);
	pr("%4d", land.lnd_item[I_IRON]);
	pr("%4d", land.lnd_item[I_DUST]);
	pr("%4d", land.lnd_item[I_BAR]);
	pr("%4d", land.lnd_item[I_OIL]);
	pr("%4d", land.lnd_item[I_LCM]);
	pr("%4d", land.lnd_item[I_HCM]);
	pr("%4d\n", land.lnd_item[I_RAD]);
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("No unit(s)\n");
	return RET_FAIL;
    } else
	pr("%d unit%s\n", nunits, splur(nunits));
    return RET_OK;
}
