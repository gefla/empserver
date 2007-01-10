/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include "budg.h"
#include "lost.h"
#include "plane.h"
#include "player.h"
#include "ship.h"
#include "update.h"

static void planerepair(struct plnstr *, struct natstr *, int *, int);
static void upd_plane(struct plnstr *, int, struct natstr *, int *, int);

int
prod_plane(int etus, int natnum, int *bp, int buildem)
		 /* Build = 1, maintain =0 */
{
    struct plnstr *pp;
    struct natstr *np;
    int n, k = 0;
    int start_money;

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

	if (pp->pln_flags & PLN_LAUNCHED) {
	    if (!player->simulation && buildem == 0
		&& !(pp->pln_flags & PLN_SYNCHRONOUS))
		move_sat(pp);
	    continue;
	}

	np = getnatp(pp->pln_own);
	start_money = np->nat_money;
	upd_plane(pp, etus, np, bp, buildem);
	air_money[pp->pln_own] += np->nat_money - start_money;
	if (buildem == 0 || np->nat_money != start_money)
	    k++;
	if (player->simulation)
	    np->nat_money = start_money;
    }

    return k;
}

static void
upd_plane(struct plnstr *pp, int etus,
	  struct natstr *np, int *bp, int build)
{
    struct plchrstr *pcp = &plchr[(int)pp->pln_type];
    int mult, cost, eff;

    if (build == 1) {
	if (!pp->pln_off && np->nat_money >= 0)
	    planerepair(pp, np, bp, etus);
	if (!player->simulation)
	    pp->pln_off = 0;
    } else {
	mult = 1;
	if (np->nat_level[NAT_TLEV] < pp->pln_tech * 0.85)
	    mult = 2;
	cost = -(mult * etus * MIN(0.0, pcp->pl_cost * money_plane));
	if (np->nat_money < cost && !player->simulation) {
	    if ((eff = pp->pln_effic - etus / 5) < PLANE_MINEFF) {
		wu(0, pp->pln_own,
		   "%s lost to lack of maintenance\n", prplane(pp));
		makelost(EF_PLANE, pp->pln_own, pp->pln_uid,
			 pp->pln_x, pp->pln_y);
		pp->pln_own = 0;
		return;
	    }
	    wu(0, pp->pln_own,
	       "%s lost %d%% to lack of maintenance\n",
	       prplane(pp), pp->pln_effic - eff);
	    pp->pln_effic = eff;
	} else {
	    np->nat_money -= cost;
	}
	/* flight pay is 5x the pay received by other military */
	np->nat_money += etus * pcp->pl_crew * money_mil * 5;
    }
}

static void
planerepair(struct plnstr *pp, struct natstr *np, int *bp, int etus)
{
    int build;
    int mvec[I_MAX + 1];
    struct shpstr *carrier;
    struct plchrstr *pcp = &plchr[(int)pp->pln_type];
    struct sctstr *sp = getsectp(pp->pln_x, pp->pln_y);
    int delta;
    int mult;
    int avail;
    int w_p_eff;
    int used;

    carrier = NULL;
    if (pp->pln_ship >= 0) {
	if (pp->pln_effic >= 80)
	    return;
	carrier = getshipp(pp->pln_ship);
	if (CANT_HAPPEN(!carrier))
	    return;
	if (carrier->shp_off)
	    return;
	if ((carrier->shp_own != pp->pln_own) &&
	    (getrel(getnatp(carrier->shp_own), pp->pln_own) != ALLIED))
	    return;
    } else {
	if ((sp->sct_own != pp->pln_own) &&
	    (getrel(getnatp(sp->sct_own), pp->pln_own) != ALLIED))
	    return;
    }

    if (sp->sct_off)
	return;
    mult = 1;
    if (np->nat_level[NAT_TLEV] < pp->pln_tech * 0.85)
	mult = 2;

    if (pp->pln_effic == 100)
	return;

    if (!player->simulation)
	avail = sp->sct_avail * 100;
    else
	avail = gt_bg_nmbr(bp, sp, I_MAX + 1) * 100;
    if (carrier)
	avail += etus * carrier->shp_item[I_MILIT] / 2;

    w_p_eff = PLN_BLD_WORK(pcp->pl_lcm, pcp->pl_hcm);
    delta = roundavg((double)avail / w_p_eff);
    if (delta <= 0)
	return;
    if (delta > (int)((float)etus * plane_grow_scale))
	delta = (int)((float)etus * plane_grow_scale);
    if (delta > 100 - pp->pln_effic)
	delta = 100 - pp->pln_effic;

    memset(mvec, 0, sizeof(mvec));
    mvec[I_MILIT] =  pcp->pl_crew;
    mvec[I_LCM] =  pcp->pl_lcm;
    mvec[I_HCM] = pcp->pl_hcm;
    build = get_materials(sp, bp, mvec, delta);

    if (carrier)
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
    if (carrier) {
	if ((pp->pln_effic + build) > 80)
	    build = 80 - pp->pln_effic;
    }

    np->nat_money -= mult * build * pcp->pl_cost / 100.0;

    if (!player->simulation)
	pp->pln_effic += (signed char)build;
}
