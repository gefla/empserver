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
 *  distribute.c: Do distribution to sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include <stdlib.h>
#include "misc.h"
#include "var.h"
#include "nat.h"
#include "sect.h"
#include "item.h"
#include "xy.h"
#include "path.h"
#include "file.h"
#include "distribute.h"
#include "update.h"
#include "subs.h"
#include "common.h"
#include "prototypes.h"

int
dodistribute(struct sctstr *sp, int imex, s_char *path, double dist_i_cost,
	     double dist_e_cost)
	  /* import or export? */
{
    struct ichrstr *ip;
    struct sctstr *dist;
    int amt;
    int thresh;
    int amt_dist;
    int amt_sect;
    int packing;
    float imcost;
    float excost;
    int dist_packing;
    int diff;
    int item;
    int changed;
    int rplague;
    int lplague;

    if ((sp->sct_dist_x == sp->sct_x) && (sp->sct_dist_y == sp->sct_y))
	return 0;

    if (path == (s_char *)0) {
	if (sp->sct_own != 0) {
	    if (imex == EXPORT)	/* only want this once */
		wu(0, sp->sct_own, "No path to dist sector for %s\n",
		   ownxy(sp));
	}
	return 0;
    }

    dist = getsectp(sp->sct_dist_x, sp->sct_dist_y);
    if (dist->sct_effic >= 60)
	dist_packing = dchr[dist->sct_type].d_pkg;
    else
	dist_packing = NPKG;	/* No packing */

    if (sp->sct_effic >= 60)
	packing = dchr[sp->sct_type].d_pkg;
    else
	packing = NPKG;		/* No packing */

    if ((dist->sct_effic >= 60) && dchr[dist->sct_type].d_pkg == WPKG)
	packing = dchr[dist->sct_type].d_pkg;

    lplague = rplague = changed = 0;
    for (item = 1; item < I_MAX + 1; item++) {
	if (sp->sct_dist[item] == 0)
	    continue;
	ip = &ichr[item];
	thresh = sp->sct_dist[item];
	/*
	 * calculate costs for importing and exporting.
	 * the div 10.0 is because delivering straight through
	 * to the dist sect is cheaper than stopping at each
	 * sector along the way (processor-timewise)
	 */
	excost = (dist_e_cost / ip->i_pkg[packing] * ip->i_lbs) / 10.0;
	imcost = (dist_i_cost / ip->i_pkg[dist_packing] * ip->i_lbs) / 10.0;
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
	    if (dist->sct_mobil <= 0) {
		/*logerror("  dist mobil < 0"); */
		continue;
	    }
	    amt = diff;
	    if (item == I_CIVIL)
		amt_dist--;	/* Don't send your last civ */

	    if (amt_dist < amt) {
		amt = amt_dist;
		if (amt_dist == 0)
		    continue;
	    }
	    if (dist->sct_mobil < imcost * amt)
		amt = dist->sct_mobil / imcost;

	    lplague++;
	    /* XXX replace with vector assign and putvec() */
	    dist->sct_item[item] -= amt;
	    changed++;
	    dist->sct_mobil -= (int)(imcost * amt);
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
	    if (sp->sct_mobil <= 0) {
		/*logerror("  sp mob is zero"); */
		continue;
	    }
	    amt = diff;
	    if (amt > amt_sect)
		amt = amt_sect;
	    if (sp->sct_mobil < excost * amt)
		amt = sp->sct_mobil / excost;
	    if (amt > ITEM_MAX - amt_dist)
		amt = ITEM_MAX - amt_dist;
	    if (amt == 0)
		continue;
	    /* XXX replace with vector assign and putvec() */

	    rplague++;
	    sp->sct_item[item] -= amt;
	    changed++;
	    sp->sct_mobil -= (int)(excost * amt);
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
