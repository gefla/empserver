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
 *  btu.c: Dealing with BTUs
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "chance.h"
#include "nat.h"
#include "optlist.h"
#include "sect.h"

/*
 * Return BTUs produced by @cap in @etu ETUs.
 */
static int
accrued_btus(struct sctstr *cap, int etu)
{
    double eff, civ;

    switch (cap->sct_type) {
    case SCT_CAPIT:
    case SCT_SANCT:
	eff = cap->sct_effic;
	break;
    case SCT_MOUNT:
	eff = 0;
	break;
    default:
	return 0;
    }

    eff *= cap->sct_work / 100.0;
    if (eff < 0.5)
	eff = 0.5;

    civ = cap->sct_item[I_CIVIL];
    if (civ > 1000)
	civ = 1000;

    return roundavg(etu * civ * eff * btu_build_rate);
}

/*
 * Grant nation @np the BTUs produced by its capital in @etu ETUs.
 * Return whether it has a capital.
 */
int
grant_btus(struct natstr *np, int etu)
{
    int has_cap, delta;
    struct sctstr sect;

    getsect(np->nat_xcap, np->nat_ycap, &sect);
    has_cap = np->nat_stat >= STAT_ACTIVE && !influx(np);

    if (has_cap) {
	delta = accrued_btus(&sect, etu);
	if (delta + np->nat_btu > max_btus)
	    np->nat_btu = max_btus;
	else
	    np->nat_btu += delta;
    }
    if (np->nat_stat == STAT_VIS || np->nat_stat == STAT_GOD)
	np->nat_btu = max_btus;

    return has_cap;
}
