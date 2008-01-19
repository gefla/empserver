/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  btu.c: Dealing with BTUs
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2007
 */

#include <config.h>

#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"
#include "sect.h"

/*
 * Return BTUs produced by CAP in ETU ETUs.
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
    if (civ > 999)
	civ = 999;

    return roundavg(etu * civ * eff * btu_build_rate);
}

/*
 * Grant nation NP the BTUs produced by its capital in ETU ETUs.
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
