/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  age.c: Age people
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "update.h"

int
age_people(int n, int etu)
{
    /* age by 1% per 24 etus */
    return roundavg(n * (1.0 - etu / 2400.0));
}

void
age_levels(int etu)
{
    float best_tech, best_res;
    struct natstr *np;
    int i;
    double level;
    double delta;

    best_tech = 0.0;
    best_res = 0.0;
    for (i = 0; NULL != (np = getnatp(i)); i++) {
	if (np->nat_stat != STAT_ACTIVE)
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
	np->nat_reserve = age_people(np->nat_reserve, etu);
    }
    best_tech /= 5;
    best_res /= 5;
    for (i = 0; NULL != (np = getnatp(i)); i++) {
	if (np->nat_stat < STAT_SANCT || np->nat_stat == STAT_GOD)
	    continue;
	level = np->nat_level[NAT_TLEV];
	if (level < best_tech && chance(0.2))
	    np->nat_level[NAT_TLEV] += (best_tech - level) / 3;
	level = np->nat_level[NAT_RLEV];
	if (level < best_res && chance(0.2))
	    np->nat_level[NAT_RLEV] += (best_res - level) / 3;
    }
}
