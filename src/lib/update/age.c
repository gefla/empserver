/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  age.c: Age people
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include "misc.h"
#include "nat.h"
#include "file.h"
#include "update.h"
#include "gen.h"

void
age_levels(int etu)
{
    extern float level_age_rate;
    register float best_tech;
    register float best_res;
    register struct natstr *np;
    int i;
    double level;
    double delta;
    int deltares;

    best_tech = 0.0;
    best_res = 0.0;
    for (i = 0; NULL != (np = getnatp(i)); i++) {
	if ((np->nat_stat & STAT_NORM) == 0)
	    continue;

	if (np->nat_stat & STAT_GOD)
	    continue;

	if (np->nat_stat == VIS)
	    continue;

	if (best_tech < np->nat_level[NAT_TLEV])
	    best_tech = np->nat_level[NAT_TLEV];
	if (best_res < np->nat_level[NAT_RLEV])
	    best_res = np->nat_level[NAT_RLEV];
	if (level_age_rate != 0.0) {
	    delta = np->nat_level[NAT_RLEV] * etu / (100 * level_age_rate);
	    np->nat_level[NAT_RLEV] -= delta;
	    delta = np->nat_level[NAT_TLEV] * etu / (100 * level_age_rate);
	    np->nat_level[NAT_TLEV] -= delta;
	}
	/*
	 * age reserves by 1% per every 24 etus
	 */
	deltares = -roundavg(np->nat_reserve * etu / 2400.0);
	if (deltares != 0)
	    np->nat_reserve += deltares;
	/* Chad Zabel - above number is negative ( was a -= there
	   which was wrong. */
    }
    best_tech /= 5;
    best_res /= 5;
    for (i = 0; NULL != (np = getnatp(i)); i++) {
	if ((np->nat_stat & STAT_INUSE) == 0)
	    continue;
	if (np->nat_stat & STAT_GOD)
	    continue;
	if (np->nat_stat == VIS)
	    continue;
	level = np->nat_level[NAT_TLEV];
	if (level < best_tech && chance(0.2))
	    np->nat_level[NAT_TLEV] += (best_tech - level) / 3;
	level = np->nat_level[NAT_RLEV];
	if (level < best_res && chance(0.2))
	    np->nat_level[NAT_RLEV] += (best_res - level) / 3;
    }
}
