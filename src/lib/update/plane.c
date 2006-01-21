/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plane.c: Do production for planes
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "sect.h"
#include "plane.h"
#include "ship.h"
#include "nat.h"
#include "file.h"
#include "optlist.h"
#include "budg.h"
#include "player.h"
#include "update.h"
#include "lost.h"
#include "subs.h"
#include "common.h"
#include "gen.h"

int
prod_plane(int etus, int natnum, int *bp, int buildem)



		 /* Build = 1, maintain =0 */
{
    struct plnstr *pp;
    struct plchrstr *plp;
    struct natstr *np;
    float leftp, buildp;
    int left, build;
    int lcm_needed, hcm_needed;
    int mil_needed;
    int mvec[I_MAX + 1];
    int n, k = 0;
    struct shpstr *shp;
    struct plchrstr *desc;
    struct sctstr *sp;
    int delta;
    int mult;
    int cost;
    int eff;
    int avail;
    int w_p_eff;
    int used;
    int start_money, onship;

    for (n = 0; NULL != (pp = getplanep(n)); n++) {
	if (pp->pln_own == 0)
	    continue;
	if (pp->pln_own != natnum)
	    continue;
	if (pp->pln_effic < PLANE_MINEFF) {
	    makelost(EF_PLANE, pp->pln_own, pp->pln_uid,
		     pp->pln_x, pp->pln_y);
	    pp->pln_own = 0;
	    continue;
	}

	plp = &plchr[(int)pp->pln_type];
	if (pp->pln_flags & PLN_LAUNCHED) {
	    if (buildem == 0) {
		if ((!player->simulation) &&
		    (plp->pl_flags & P_O) &&
		    (pp->pln_flags & PLN_LAUNCHED) &&
		    !(plp->pl_flags & P_M) &&
		    !(pp->pln_flags & PLN_SYNCHRONOUS))
		    move_sat(pp);
	    }
	    continue;
	}

	onship = 0;
	shp = (struct shpstr *)0;
	if (pp->pln_ship >= 0 && (buildem == 1)) {
	    if (pp->pln_effic >= 80)
		continue;
	    onship = 1;
	    shp = getshipp(pp->pln_ship);
	    if (shp == 0 || shp->shp_own != pp->pln_own) {
		/* nplane is unsigned... */
		if (shp->shp_nplane > 0)
		    shp->shp_nplane--;
		makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x,
			 pp->pln_y);
		pp->pln_own = 0;
		continue;
	    }
	}
	np = getnatp(pp->pln_own);
	desc = &plchr[(int)pp->pln_type];
	sp = getsectp(pp->pln_x, pp->pln_y);
	mult = 1;
	if (np->nat_level[NAT_TLEV] < pp->pln_tech * 0.85)
	    mult = 2;

	if (buildem == 0) {
	    /* flight pay is 5x the pay received by other military */
	    start_money = np->nat_money;
	    cost = -(mult * etus * MIN(0.0, desc->pl_cost * money_plane));
	    if ((np->nat_priorities[PRI_PMAINT] == 0 ||
		 np->nat_money < cost) && !player->simulation) {
		if ((eff = pp->pln_effic - etus / 5) < PLANE_MINEFF) {
		    wu(0, pp->pln_own,
		       "%s lost to lack of maintenance\n", prplane(pp));
		    makelost(EF_PLANE, pp->pln_own, pp->pln_uid, pp->pln_x,
			     pp->pln_y);
		    pp->pln_own = 0;
		    continue;
		}
		wu(0, pp->pln_own,
		   "%s lost %d%% to lack of maintenance\n",
		   prplane(pp), pp->pln_effic - eff);
		pp->pln_effic = eff;
	    } else {
		np->nat_money -= cost;
	    }

	    np->nat_money += (etus * plp->pl_crew * money_mil * 5);

	    air_money[pp->pln_own] += np->nat_money - start_money;
	    k++;
	    if (player->simulation)
		np->nat_money = start_money;
	    if ((pp->pln_flags & PLN_LAUNCHED) == PLN_LAUNCHED)
		continue;
	} else {
	    if (sp->sct_off)
		continue;
	    if (np->nat_priorities[PRI_PBUILD] == 0 || np->nat_money < 0)
		continue;

	    start_money = np->nat_money;
	    left = 100 - pp->pln_effic;
	    if (left <= 0)
		continue;

	    if (!player->simulation)
		avail = sp->sct_avail * 100;
	    else
		avail = gt_bg_nmbr(bp, sp, I_MAX + 1) * 100;

	    if (pp->pln_ship >= 0) {
		shp = getshipp(pp->pln_ship);
		avail += (etus * shp->shp_item[I_MILIT] / 2);
	    }
	    w_p_eff = PLN_BLD_WORK(desc->pl_lcm, desc->pl_hcm);
	    delta = roundavg((double)avail / w_p_eff);
	    if (delta <= 0)
		continue;
	    if (delta > (int)((float)etus * plane_grow_scale))
		delta = (int)((float)etus * plane_grow_scale);
	    if (delta > left)
		delta = left;

	    /* delta is the max amount we can grow */

	    left = 100 - pp->pln_effic;
	    if (left > delta)
		left = delta;

	    leftp = ((float)left / 100.0);
	    memset(mvec, 0, sizeof(mvec));
	    mvec[I_MILIT] = mil_needed =
		ldround((double)(plp->pl_crew * leftp), 1);
	    mvec[I_LCM] = lcm_needed =
		ldround((double)(plp->pl_lcm * leftp), 1);
	    mvec[I_HCM] = hcm_needed =
		ldround((double)(plp->pl_hcm * leftp), 1);

	    get_materials(sp, bp, mvec, 0);

	    if (mvec[I_MILIT] >= mil_needed)
		buildp = leftp;
	    else
		buildp = ((float)mvec[I_MILIT] / (float)plp->pl_crew);

	    if (mvec[I_LCM] < lcm_needed)
		buildp = MIN(buildp, ((float)mvec[I_LCM] /
				      (float)plp->pl_lcm));

	    if (mvec[I_HCM] < hcm_needed)
		buildp = MIN(buildp, ((float)mvec[I_HCM] /
				      (float)plp->pl_hcm));

	    build = ldround((double)(buildp * 100.0), 1);
	    memset(mvec, 0, sizeof(mvec));
	    mvec[I_MILIT] = mil_needed =
		roundavg((double)(plp->pl_crew * buildp));
	    mvec[I_LCM] = lcm_needed =
		roundavg((double)(plp->pl_lcm * buildp));
	    mvec[I_HCM] = hcm_needed =
		roundavg((double)(plp->pl_hcm * buildp));

	    get_materials(sp, bp, mvec, 1);

	    if (onship)
		build = delta;
	    used = build * w_p_eff;

	    /*
	     * I didn't use roundavg here, because I want to
	     * penalize the player with a large number of planes.
	     */
	    if (!player->simulation)
		avail = (sp->sct_avail * 100 - used) / 100;
	    else
		avail = (gt_bg_nmbr(bp, sp, I_MAX + 1) * 100 - used) / 100;

	    if (avail < 0)
		avail = 0;
	    if (!player->simulation)
		sp->sct_avail = avail;
	    else
		pt_bg_nmbr(bp, sp, I_MAX + 1, avail);

	    if (sp->sct_type != SCT_AIRPT)
		build /= 3;
	    if (onship) {
		if ((pp->pln_effic + build) > 80)
		    build = 80 - pp->pln_effic;
	    }
	    np->nat_money -= roundavg(mult * build *
				      desc->pl_cost / 100.0);
	    air_money[pp->pln_own] += np->nat_money - start_money;

	    if (!player->simulation)
		pp->pln_effic += (s_char)build;
	    else
		np->nat_money = start_money;
	    k++;
	}
    }
    return k;
}
