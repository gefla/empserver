/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  res_pop.c: Get maximum pop of a sector
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include <config.h>

#include "item.h"
#include "misc.h"
#include "optlist.h"
#include "prototypes.h"
#include "sect.h"
#include "xy.h"

int
max_population(float research, int desig, int eff)
{
    int maxpop = dchr[desig].d_maxpop;
    int mp100, rmax;

    /* city efficiency limits maximum population */
    if (IS_BIG_CITY(desig)) {
	mp100 = MIN(ITEM_MAX, 10 * maxpop);
	maxpop += (mp100 - maxpop) * eff / 100.0;
    }

    if (opt_RES_POP) {
	/* research limits maximum population */
	rmax = maxpop * 0.4
	    + maxpop * 0.6 * (50.0 + 4.0*research) / (200.0 + 3.0*research);
	if (maxpop > rmax)
	    maxpop = rmax;
    }

    if (CANT_HAPPEN(maxpop > ITEM_MAX))
	maxpop = ITEM_MAX;

    return maxpop;
}

int
max_pop(float research, struct sctstr *sp)
{
    return max_population(research, sp->sct_type, sp->sct_effic);
}
