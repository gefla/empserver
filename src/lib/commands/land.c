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
 *  land.c: List land units
 *
 *  Known contributors to this file:
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2013
 */

#include <config.h>

#include "commands.h"
#include "land.h"

int
land(void)
{
    int nunits, noff;
    struct nstr_item ni;
    struct lndstr land;

    if (!snxtitem(&ni, EF_LAND, player->argp[1], NULL))
	return RET_SYN;

    nunits = noff = 0;
    while (nxtitem(&ni, &land)) {
	if (land.lnd_own == 0)
	    continue;
	if (!player->owner && !player->god)
	    continue;
	if (nunits++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("   # unit type          x,y    a   eff mil frt  mu  fd"
	       " tch retr xl ln carry\n");
	}
	if (land.lnd_off)
	    noff++;
	if (player->god)
	    pr("%3d ", land.lnd_own);
	pr("%4d ", ni.cur);
	pr("%-15.15s", lchr[(int)land.lnd_type].l_name);
	prxy(" %4d,%-4d ", land.lnd_x, land.lnd_y);
	pr("%1.1s", &land.lnd_army);
	pr(" %c%3d%%", land.lnd_off ? '!' : ' ', land.lnd_effic);
	pr("%4d", land.lnd_item[I_MILIT]);
	pr("%4d", land.lnd_harden);
	pr("%4d", land.lnd_mobil);
	pr("%4d", land.lnd_item[I_FOOD]);
	pr("%4d ", land.lnd_tech);
	pr("%3d%%", land.lnd_retreat);
	pr("%3d", lnd_nxlight(&land));
	pr("%3d", lnd_nland(&land));
	if (land.lnd_ship >= 0)
	    pr(" %4dS", land.lnd_ship);
	else if (land.lnd_land >= 0)
	    pr(" %4dL", land.lnd_land);
	pr("\n");
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("%s: No unit(s)\n", "");
	return RET_FAIL;
    } else {
	pr("%d unit%s", nunits, splur(nunits));
	if (noff)
	    pr(", %d stopped (eff marked with !)", noff);
	pr("\n");
    }
    return RET_OK;
}
