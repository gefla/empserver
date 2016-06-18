/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plane.c: Do production for planes
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include "chance.h"
#include "file.h"
#include "lost.h"
#include "nat.h"
#include "optlist.h"
#include "plane.h"
#include "player.h"
#include "prototypes.h"
#include "ship.h"
#include "update.h"

static void planerepair(struct plnstr *, struct natstr *, struct bp *, int);
static void upd_plane(struct plnstr *, int, struct natstr *, struct bp *, int);

int
prod_plane(int etus, int natnum, struct bp *bp, int buildem)
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

	if (pln_is_in_orbit(pp)) {
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
	  struct natstr *np, struct bp *bp, int build)
{
    struct plchrstr *pcp = &plchr[(int)pp->pln_type];
    int mult, cost, eff_lost;

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
	    eff_lost = etus / 5;
	    if (pp->pln_effic - eff_lost < PLANE_MINEFF)
		eff_lost = pp->pln_effic - PLANE_MINEFF;
	    if (eff_lost > 0) {
		wu(0, pp->pln_own, "%s lost %d%% to lack of maintenance\n",
		   prplane(pp), eff_lost);
		pp->pln_effic -= eff_lost;
	    }
	} else {
	    np->nat_money -= cost;
	}
	/* flight pay is 5x the pay received by other military */
	np->nat_money += etus * pcp->pl_mat[I_MILIT] * money_mil * 5;
    }
}

static void
planerepair(struct plnstr *pp, struct natstr *np, struct bp *bp, int etus)
{
    struct plchrstr *pcp = &plchr[(int)pp->pln_type];
    int build;
    struct shpstr *carrier;
    struct sctstr *sp, scratch_sect;
    int delta;
    int mult;
    int avail;
    int used;

    if (pp->pln_effic == 100)
	return;

    sp = getsectp(pp->pln_x, pp->pln_y);
    if (sp->sct_off)
	return;

    carrier = NULL;
    if (pp->pln_ship >= 0) {
	if (pp->pln_effic >= 80)
	    return;
	carrier = getshipp(pp->pln_ship);
	if (CANT_HAPPEN(!carrier))
	    return;
	if (carrier->shp_off)
	    return;
	if (relations_with(carrier->shp_own, pp->pln_own) != ALLIED)
	    return;
    } else {
	if (relations_with(sp->sct_own, pp->pln_own) != ALLIED)
	    return;
    }

    if (player->simulation) {
	scratch_sect = *sp;
	bp_to_sect(bp, &scratch_sect);
	sp = &scratch_sect;
    }

    mult = 1;
    if (np->nat_level[NAT_TLEV] < pp->pln_tech * 0.85)
	mult = 2;

    avail = sp->sct_avail * 100;
    if (carrier)
	avail += etus * carrier->shp_item[I_MILIT] / 2;

    delta = avail / pcp->pl_bwork;
    if (delta <= 0)
	return;
    if (delta > (int)((float)etus * plane_grow_scale))
	delta = (int)((float)etus * plane_grow_scale);
    if (delta > 100 - pp->pln_effic)
	delta = 100 - pp->pln_effic;

    build = get_materials(sp, pcp->pl_mat, delta);

    if (carrier)
	build = delta;

    used = build * pcp->pl_bwork;
    avail = roundavg((sp->sct_avail * 100 - used) / 100.0);
    if (avail < 0)
	avail = 0;
    sp->sct_avail = avail;

    if (sp->sct_type != SCT_AIRPT)
	build /= 3;
    if (carrier) {
	if ((pp->pln_effic + build) > 80)
	    build = 80 - pp->pln_effic;
    }

    bp_set_from_sect(bp, sp);
    np->nat_money -= roundavg(mult * build * pcp->pl_cost / 100.0);

    if (!player->simulation)
	pp->pln_effic += (signed char)build;
}
