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
 *  distribute.c: Do distribution to sectors
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 *     Markus Armbruster, 2004-2011
 */

#include <config.h>

#include "item.h"
#include "optlist.h"
#include "path.h"
#include "plague.h"
#include "prototypes.h"
#include "sect.h"
#include "update.h"

#ifdef DISTRIBUTE_DEBUG
#define DPRINTF(fmt, ...) ((void)printf(fmt , ## __VA_ARGS__))
#else
#define DPRINTF(fmt, ...) ((void)0)
#endif

#define EXPORT_BONUS 10.0
#define IMPORT_BONUS 10.0

int
dodistribute(struct sctstr *sp, int imex, double import_cost)
{
    double path_cost, dcc;
    struct ichrstr *ip;
    struct sctstr *dist;
    int amt;
    int thresh;
    int amt_dist;
    int amt_sect;
    enum i_packing sect_packing, dist_packing;
    int pack;
    double mcost;
    int diff;
    i_type item;
    int changed;
    int rplague;
    int lplague;

    if (imex == IMPORT && import_cost < 0.0)
	return 0;

    dist = getsectp(sp->sct_dist_x, sp->sct_dist_y);
    if (imex == IMPORT)
	path_cost = import_cost;
    else {
	dcc = sector_mcost(dist, MOB_MOVE);
	if (import_cost < 0.0 || dcc < 0.0) {
	    if (sp->sct_own && sp->sct_uid != dist->sct_uid)
		wu(0, sp->sct_own, "No path to dist sector for %s\n",
		   ownxy(sp));
	    return 0;
	}
	path_cost = import_cost - sector_mcost(sp, MOB_MOVE) + dcc;
    }

    dist_packing = dist->sct_effic >= 60 ? dchr[dist->sct_type].d_pkg : IPKG;
    sect_packing = sp->sct_effic   >= 60 ? dchr[sp->sct_type].d_pkg : IPKG;

    DPRINTF("distribute: %d,%d to %d,%d pathcost %g\n",
	    sp->sct_x, sp->sct_y, sp->sct_dist_x, sp->sct_dist_y,
	    path_cost);

    lplague = rplague = changed = 0;
    for (item = I_NONE + 1; item <= I_MAX; item++) {
	if (sp->sct_dist[item] == 0)
	    continue;
	ip = &ichr[item];
	thresh = sp->sct_dist[item];
	/*
	 * calculate costs for importing and exporting.
	 * the mob bonus is because delivering straight through
	 * to the dist sect is cheaper than stopping at each
	 * sector along the way (processor-timewise)
	 */
	amt_sect = sp->sct_item[item];
	amt_dist = dist->sct_item[item];
	diff = amt_sect - thresh;
	if (item == I_CIVIL)
	    if (sp->sct_own != sp->sct_oldown)
		continue;
	if (item == I_CIVIL)
	    if (dist->sct_own != dist->sct_oldown)
		continue;
	if (diff < 0) {
	    if (imex != IMPORT)
		continue;

	    if (!military_control(dist))
		continue;

	    diff = -diff;
	    /*
	     * import.
	     * don't import if no mobility.
	     * check to make sure have enough mobility in the
	     * dist sector to import what we need.
	     */
	    if (dist->sct_mobil <= 0)
		continue;
	    amt = diff;
	    /* make sure not to abandon the sector */
	    if (item == I_CIVIL ||
		(item == I_MILIT && dist->sct_item[I_CIVIL] == 0))
		amt_dist--;

	    if (amt_dist < amt) {
		amt = amt_dist;
		if (amt_dist <= 0)
		    continue;
	    }
	    pack = ip->i_pkg[dist_packing];
	    mcost = path_cost / pack * ip->i_lbs / IMPORT_BONUS;
	    if (dist->sct_mobil < mcost * amt)
		amt = dist->sct_mobil / mcost;

	    lplague++;
	    dist->sct_item[item] -= amt;
	    changed++;
	    dist->sct_mobil -= (int)(mcost * amt);
	    sp->sct_item[item] += amt;
	} else {
	    if (imex != EXPORT)
		continue;
	    if (!military_control(sp))
		continue;
	    if ((item == I_CIVIL) && (sp->sct_work < 100))
		continue;
	    if ((item == I_CIVIL) && (sp->sct_own != sp->sct_oldown))
		continue;
	    /*
	     * export.
	     * don't export if no mobility. check to make sure we
	     * have mobility enough to do the right thing.
	     * also make sure that there's enough space in the
	     * target sector to hold the required amt.
	     */
	    if (sp->sct_mobil <= 0)
		continue;
	    amt = diff;
	    if (amt > amt_sect)
		amt = amt_sect;
	    pack = MAX(ip->i_pkg[sect_packing], ip->i_pkg[dist_packing]);
	    mcost = path_cost / pack * ip->i_lbs / EXPORT_BONUS;
	    if (sp->sct_mobil < mcost * amt)
		amt = sp->sct_mobil / mcost;
	    if (amt > ITEM_MAX - amt_dist)
		amt = ITEM_MAX - amt_dist;
	    if (amt == 0)
		continue;

	    rplague++;
	    sp->sct_item[item] -= amt;
	    changed++;
	    sp->sct_mobil -= (int)(mcost * amt);
	    dist->sct_item[item] += amt;
	}
    }

    if (lplague) {
	lplague = dist->sct_pstage;
	if (lplague == PLG_INFECT && sp->sct_pstage == PLG_HEALTHY)
	    sp->sct_pstage = PLG_EXPOSED;
    }

    if (rplague) {
	rplague = sp->sct_pstage;
	if (rplague == PLG_INFECT && dist->sct_pstage == PLG_HEALTHY)
	    dist->sct_pstage = PLG_EXPOSED;
    }

    return changed;
}
