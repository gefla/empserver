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
 *  sect.c: Do production for sectors
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "budg.h"
#include "chance.h"
#include "item.h"
#include "land.h"
#include "lost.h"
#include "path.h"
#include "player.h"
#include "ship.h"
#include "update.h"

int
buildeff(struct sctstr *sp)
{
    int work_cost = 0;
    int avail = sp->sct_avail;
    int buildeff_work = avail / 2;
    int cost, n, hcms, lcms, neweff, desig;

    cost = 0;
    neweff = sp->sct_effic;

    if (sp->sct_type != sp->sct_newtype) {
	/*
	 * Tear down existing sector.
	 * Easier to destroy than to build.
	 */
	work_cost = (sp->sct_effic + 3) / 4;
	if (work_cost > buildeff_work)
	    work_cost = buildeff_work;
	buildeff_work -= work_cost;
	n = sp->sct_effic - work_cost * 4;
	if (n <= 0) {
	    n = 0;
	    sp->sct_type = sp->sct_newtype;
	}
	neweff = n;
	cost += work_cost;
    }

    desig = sp->sct_type;
    if (desig == sp->sct_newtype) {
	work_cost = 100 - neweff;
	if (work_cost > buildeff_work)
	    work_cost = buildeff_work;

	if (dchr[desig].d_lcms > 0) {
	    lcms = sp->sct_item[I_LCM];
	    lcms /= dchr[desig].d_lcms;
	    if (work_cost > lcms)
		work_cost = lcms;
	}
	if (dchr[desig].d_hcms > 0) {
	    hcms = sp->sct_item[I_HCM];
	    hcms /= dchr[desig].d_hcms;
	    if (work_cost > hcms)
		work_cost = hcms;
	}

	neweff += work_cost;
	cost += work_cost * dchr[desig].d_build;
	buildeff_work -= work_cost;

	if ((dchr[desig].d_lcms > 0) || (dchr[desig].d_hcms > 0)) {
	    sp->sct_item[I_LCM] -= work_cost * dchr[desig].d_lcms;
	    sp->sct_item[I_HCM] -= work_cost * dchr[desig].d_hcms;
	}
    }

    sp->sct_effic = neweff;
    sp->sct_avail = (avail + 1) / 2 + buildeff_work;
    return cost;
}

/*
 * enlistment sectors are special; they require military
 * to convert civ into mil in large numbers.
 * Conversion will happen much more slowly without
 * some mil initially.
 */
static int
enlist(short *vec, int etu, int *cost)
{
    int maxmil;
    int enlisted;

    enlisted = 0;
    maxmil = vec[I_CIVIL] / 2 - vec[I_MILIT];
    if (maxmil > 0) {
	enlisted = etu * (10 + vec[I_MILIT]) * 0.05;
	if (enlisted > maxmil)
	    enlisted = maxmil;
	vec[I_CIVIL] -= enlisted;
	vec[I_MILIT] += enlisted;
    }
    *cost = enlisted * 3;
    return enlisted;
}

/* Fallout is calculated here. */

static void
meltitems(int etus, int fallout, int own, short *vec,
	  int type, int x, int y, int uid)
{
    i_type n;
    int melt;

    for (n = I_NONE + 1; n <= I_MAX; n++) {
	melt = roundavg(vec[n] * etus * (double)fallout
			/ (1000.0 * ichr[n].i_melt_denom));
	if (melt > vec[n])
	    melt = vec[n];
	if (melt > 5 && own) {
	    if (type == EF_SECTOR)
		wu(0, own, "Lost %d %s to radiation in %s.\n",
		   melt, ichr[n].i_name,
		   xyas(x, y, own));
	    else if (type == EF_LAND)
		wu(0, own, "Unit #%d lost %d %s to radiation in %s.\n",
		   uid, melt, ichr[n].i_name,
		   xyas(x, y, own));
	    else if (type == EF_SHIP)
		wu(0, own, "Ship #%d lost %d %s to radiation in %s.\n",
		   uid, melt, ichr[n].i_name,
		   xyas(x, y, own));
	}
	vec[n] -= melt;
    }
}

/*
 * Do fallout meltdown for sector @sp.
 * @etus above 24 are treated as 24 to avoid *huge* kill offs in
 * large ETU games.
 */
void
do_fallout(struct sctstr *sp, int etus)
{
    struct shpstr *spp;
    struct lndstr *lp;
    int i;

/* This check shouldn't be needed, but just in case. :) */
    if (!sp->sct_fallout || !sp->sct_updated)
	return;
    if (etus > 24)
	etus = 24;
    meltitems(etus, sp->sct_fallout, sp->sct_own, sp->sct_item,
	      EF_SECTOR, sp->sct_x, sp->sct_y, 0);
    for (i = 0; NULL != (lp = getlandp(i)); i++) {
	if (!lp->lnd_own)
	    continue;
	if (lp->lnd_x != sp->sct_x || lp->lnd_y != sp->sct_y)
	    continue;
	meltitems(etus, sp->sct_fallout, lp->lnd_own, lp->lnd_item,
		  EF_LAND, lp->lnd_x, lp->lnd_y, lp->lnd_uid);
    }
    for (i = 0; NULL != (spp = getshipp(i)); i++) {
	if (!spp->shp_own)
	    continue;
	if (spp->shp_x != sp->sct_x || spp->shp_y != sp->sct_y)
	    continue;
	if (mchr[(int)spp->shp_type].m_flags & M_SUB)
	    continue;
	meltitems(etus, sp->sct_fallout, spp->shp_own, spp->shp_item,
		  EF_SHIP, spp->shp_x, spp->shp_y, spp->shp_uid);
    }
}

void
spread_fallout(struct sctstr *sp, int etus)
{
    struct sctstr *ap;
    int n;
    int inc;

    if (etus > 24)
	etus = 24;
    for (n = DIR_FIRST; n <= DIR_LAST; n++) {
	ap = getsectp(sp->sct_x + diroff[n][0], sp->sct_y + diroff[n][1]);
	if (ap->sct_type == SCT_SANCT)
	    continue;
	inc = roundavg(etus * fallout_spread * (sp->sct_fallout)) - 1;
	if (inc < 0)
	    inc = 0;
	ap->sct_fallout = MIN(ap->sct_fallout + inc, FALLOUT_MAX);
    }
}

void
decay_fallout(struct sctstr *sp, int etus)
{
    int decay;

    if (etus > 24)
	etus = 24;
    decay = roundavg((decay_per_etu + 6.0) * fallout_spread *
		     (double)etus * (double)sp->sct_fallout);

    sp->sct_fallout = decay < sp->sct_fallout ? sp->sct_fallout - decay : 0;
}

/*
 * Produce for a specific nation
 */
void
produce_sect(struct natstr *np, int etu, struct bp *bp, int p_sect[][2])
{
    struct sctstr *sp, scratch_sect;
    int work, cost, ecost, pcost;
    int n, amount;

    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (sp->sct_type == SCT_WATER)
	    continue;
	if (sp->sct_own != np->nat_cnum)
	    continue;
	if (sp->sct_updated != 0)
	    continue;

	/*
	 * When running the test suite, reseed PRNG for each sector
	 * with its UID, to keep results stable even when the number
	 * of PRNs consumed changes.
	 */
	if (running_test_suite)
	    seed_prng(sp->sct_uid);

	if (player->simulation) {
	    /* work on a copy, which will be discarded */
	    scratch_sect = *sp;
	    sp = &scratch_sect;
	}

	/* If everybody is dead, the sector reverts to unowned.
	 * This is also checked at the end of the production in
	 * they all starved or were plagued off.
	 */
	if (sp->sct_item[I_CIVIL] == 0 && sp->sct_item[I_MILIT] == 0 &&
	    !has_units(sp->sct_x, sp->sct_y, sp->sct_own)) {
	    if (!player->simulation) {
		makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
		sp->sct_own = 0;
		sp->sct_oldown = 0;
	    }
	    continue;
	}

	sp->sct_updated = 1;

	work = do_feed(sp, np, etu, 0);
	bp_put_items(bp, sp);

	if (sp->sct_off || np->nat_money < 0)
	    continue;

	sp->sct_avail = work;
	amount = 0;
	pcost = cost = ecost = 0;

	if (dchr[sp->sct_type].d_maint) {
	    cost = etu * dchr[sp->sct_type].d_maint;
	    p_sect[SCT_MAINT][0]++;
	    p_sect[SCT_MAINT][1] += cost;
	    if (!player->simulation)
		np->nat_money -= cost;
	}

	if ((sp->sct_effic < 100 || sp->sct_type != sp->sct_newtype) &&
	    np->nat_money >= 0) {
	    cost = buildeff(sp);
	    bp_put_items(bp, sp);
	    p_sect[SCT_EFFIC][0]++;
	    p_sect[SCT_EFFIC][1] += cost;
	    if (!player->simulation)
		np->nat_money -= cost;
	}

	if (sp->sct_type == SCT_ENLIST && sp->sct_effic >= 60 &&
	    sp->sct_own == sp->sct_oldown) {
	    p_sect[sp->sct_type][0] += enlist(sp->sct_item, etu, &ecost);
	    p_sect[sp->sct_type][1] += ecost;
	    if (!player->simulation)
		np->nat_money -= ecost;
	    bp_put_items(bp, sp);
	}

	/*
	 * now do the production (if sector effic >= 60%)
	 */

	if (sp->sct_effic >= 60) {
	    if (np->nat_money >= 0 && dchr[sp->sct_type].d_prd >= 0)
		amount = produce(np, sp, &pcost);
	    bp_put_items(bp, sp);
	}

	bp_put_avail(bp, sp, sp->sct_avail);
	p_sect[sp->sct_type][0] += amount;
	p_sect[sp->sct_type][1] += pcost;
	if (!player->simulation)
	    np->nat_money -= pcost;
    }
}
