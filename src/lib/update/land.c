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
 *  land.c: Do production for land units
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include "chance.h"
#include "land.h"
#include "lost.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "plague.h"
#include "player.h"
#include "prototypes.h"
#include "update.h"

static void upd_land(struct lndstr *, int, struct bp *, int);
static void plague_land(struct lndstr *, int);
static void landrepair(struct lndstr *, struct natstr *, struct bp *,
		       int, struct budget *);

void prep_lands(int etus, struct bp *bp)
{
    int mil, i, n;
    double mil_pay;
    struct lndstr *lp;

    for (i = 0; (lp = getlandp(i)); i++) {
	if (lp->lnd_own == 0)
	    continue;
	if (CANT_HAPPEN(lp->lnd_effic < LAND_MINEFF)) {
	    makelost(EF_LAND, lp->lnd_own, lp->lnd_uid,
		     lp->lnd_x, lp->lnd_y);
	    lp->lnd_own = 0;
	    continue;
	}

	bp_consider_unit(bp, (struct empobj *)lp);
	mil = lp->lnd_item[I_MILIT];
	mil_pay = mil * etus * money_mil;
	nat_budget[lp->lnd_own].mil.count += mil;
	nat_budget[lp->lnd_own].mil.money += mil_pay;
	nat_budget[lp->lnd_own].money += mil_pay;

	if (!player->simulation) {
	    if ((n = feed_people(lp->lnd_item, etus)) > 0) {
		wu(0, lp->lnd_own, "%d starved in %s\n", n, prland(lp));
		if (n > 10)
		    nreport(lp->lnd_own, N_DIE_FAMINE, 0, 1);
	    }
	    plague_land(lp, etus);
	}
    }
}

void
prod_land(int etus, struct bp *bp, int build)
		/* build = 1, maintain = 0 */
{
    struct lndstr *lp;
    int i;

    for (i = 0; (lp = getlandp(i)); i++) {
	if (lp->lnd_own == 0)
	    continue;
	if (bp_skip_unit(bp, (struct empobj *)lp))
	    continue;
	upd_land(lp, etus, bp, build);
    }
}

static void
upd_land(struct lndstr *lp, int etus, struct bp *bp, int build)
	       /* build = 1, maintain = 0 */
{
    struct budget *budget = &nat_budget[lp->lnd_own];
    struct lchrstr *lcp = &lchr[lp->lnd_type];
    struct natstr *np = getnatp(lp->lnd_own);
    int min = morale_base - (int)np->nat_level[NAT_HLEV];
    int mult, eff_lost;
    double cost;

    if (!player->simulation)
	if (lp->lnd_retreat < min)
	    lp->lnd_retreat = min;

    if (build == 1) {
	if (!lp->lnd_off && budget->money >= 0)
	    landrepair(lp, np, bp, etus, budget);
	if (!player->simulation)
	    lp->lnd_off = 0;
    } else {
	budget->oldowned_civs += lp->lnd_item[I_CIVIL];
	mult = 1;
	if (np->nat_level[NAT_TLEV] < lp->lnd_tech * 0.85)
	    mult = 2;
	if (lcp->l_flags & L_ENGINEER)
	    mult *= 3;
	budget->bm[BUDG_LND_MAINT].count++;
	cost = mult * etus * -money_land * lcp->l_cost;
	if (budget->money < cost && !player->simulation) {
	    eff_lost = etus / 5;
	    if (lp->lnd_effic - eff_lost < LAND_MINEFF)
		eff_lost = lp->lnd_effic - LAND_MINEFF;
	    if (eff_lost > 0) {
		wu(0, lp->lnd_own, "%s lost %d%% to lack of maintenance\n",
		   prland(lp), eff_lost);
		lp->lnd_effic -= eff_lost;
	    }
	} else {
	    budget->bm[BUDG_LND_MAINT].money -= cost;
	    budget->money -= cost;
	}
    }
}

void
plague_land(struct lndstr *lp, int etus)
{
    struct natstr *np = getnatp(lp->lnd_own);
    int pstage, ptime;
    int n;

    /* Plague can't break out on land units, but it can still kill people */
    pstage = lp->lnd_pstage;
    ptime = lp->lnd_ptime;
    if (pstage != PLG_HEALTHY) {
	n = plague_people(np, lp->lnd_item, &pstage, &ptime, etus);
	if (n != PLG_HEALTHY)
	    plague_report(lp->lnd_own, n, pstage, ptime, etus,
			  "on", prland(lp));
	lp->lnd_pstage = pstage;
	lp->lnd_ptime = ptime;
    }
}

static void
landrepair(struct lndstr *land, struct natstr *np, struct bp *bp, int etus,
	   struct budget *budget)
{
    struct lchrstr *lp = &lchr[(int)land->lnd_type];
    int delta;
    struct sctstr *sp, scratch_sect;
    int build;
    int avail;
    int mult;
    double cost;

    if (land->lnd_effic == 100)
	return;

    sp = getsectp(land->lnd_x, land->lnd_y);
    if (sp->sct_off)
	return;

    if (relations_with(sp->sct_own, land->lnd_own) != ALLIED)
	return;

    if (player->simulation) {
	scratch_sect = *sp;
	bp_to_sect(bp, &scratch_sect);
	sp = &scratch_sect;
    }

    mult = 1;
    if (np->nat_level[NAT_TLEV] < land->lnd_tech * 0.85)
	mult = 2;

    avail = sp->sct_avail * 100;

    delta = avail / lp->l_bwork;
    if (delta <= 0)
	return;
    if (delta > (int)((float)etus * land_grow_scale))
	delta = (int)((float)etus * land_grow_scale);
    if (delta > 100 - land->lnd_effic)
	delta = 100 - land->lnd_effic;

    build = get_materials(sp, lp->l_mat, delta);

    if ((sp->sct_type != SCT_HEADQ) && (sp->sct_type != SCT_FORTR))
	build /= 3;

    avail = roundavg((avail - build * lp->l_bwork) / 100.0);
    if (avail < 0)
	avail = 0;
    sp->sct_avail = avail;

    bp_set_from_sect(bp, sp);
    cost = mult * lp->l_cost * build / 100.0;
    budget->bm[BUDG_LND_BUILD].count += !!build;
    budget->bm[BUDG_LND_BUILD].money -= cost;
    budget->money -= cost;
    if (!player->simulation)
	land->lnd_effic += (signed char)build;
}
