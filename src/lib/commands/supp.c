/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  supp.c: supply land units
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "commands.h"
#include "land.h"

int
supp(void)
{
    int nunits;
    struct nstr_item ni;
    struct lndstr land;

    if (!snxtitem(&ni, EF_LAND, player->argp[1], NULL))
	return RET_SYN;

    nunits = 0;
    while (nxtitem(&ni, &land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	nunits++;
	if (lnd_supply_all(&land))
	    pr("%s has supplies\n", prland(&land));
	else
	    pr("%s is out of supply\n", prland(&land));
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("%s: No unit(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d unit%s\n", nunits, splur(nunits));
    return RET_OK;
}
