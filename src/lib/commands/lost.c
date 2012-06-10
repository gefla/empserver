/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  lost.c: List lost items
 *
 *  Known contributors to this file:
 *     Steve McClure, 1997
 */

#include <config.h>

#include "commands.h"
#include "lost.h"

int
lost(void)
{
    int nlost;
    struct nstr_item ni;
    struct loststr lost;
    time_t now;

    if (!snxtitem(&ni, EF_LOST,
		  player->argp[1] ? player->argp[1] : "*",
		  NULL))
	return RET_SYN;

    prdate();
    nlost = 0;
    time(&now);
    pr("DUMP LOST ITEMS %ld\n", (long)now);
    if (player->god)
	pr("owner ");
    pr("type id x y timestamp\n");
    while (nxtitem(&ni, &lost)) {
	if (lost.lost_owner == 0)
	    continue;
	if (lost.lost_owner != player->cnum && !player->god)
	    continue;
	if (player->god)
	    pr("%d ", lost.lost_owner);
	pr("%d %d ", lost.lost_type, lost.lost_id);
	prxy("%d %d ", lost.lost_x, lost.lost_y);
	pr("%ld\n", (long)lost.lost_timestamp);
	nlost++;
    }
    if (nlost == 0) {
	if (player->argp[1])
	    pr("%s: Nothing lost.\n", player->argp[1]);
	else
	    pr("%s: Nothing lost.\n", "");
    } else
	pr("%d lost item%s.\n", nlost, splur(nlost));
    return RET_OK;
}
