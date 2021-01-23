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
 *  material.c: Tries to find materials for production
 *
 *  Known contributors to this file:
 *     Ville Virrankoski, 1996
 *     Markus Armbruster, 2007-2016
 */

#include <config.h>

#include "chance.h"
#include "sect.h"
#include "update.h"

/*
 * Get build materials from sector @sp.
 * Array @mvec[ITEM_MAX+1] defines the materials needed to build 100%.
 * @pct is the percentage to build.
 * Adjust build percentage downwards so that available materials
 * suffice.  Remove the materials.
 * Return adjusted build percentage.
 */
int
get_materials(struct sctstr *sp, short mvec[], int pct)
{
    int i, amt;

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	if (mvec[i] == 0)
	    continue;
	amt = sp->sct_item[i];
	if (amt * 100 < mvec[i] * pct)
	    pct = amt * 100 / mvec[i];
    }

    for (i = I_NONE + 1; i <= I_MAX; i++) {
	if (mvec[i] == 0)
	    continue;
	amt = sp->sct_item[i];
	amt -= roundavg(mvec[i] * pct / 100.0);
	if (CANT_HAPPEN(amt < 0))
	    amt = 0;
	sp->sct_item[i] = amt;
    }

    return pct;
}
