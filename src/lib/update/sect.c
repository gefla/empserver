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
 *  sect.c: Do production for sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include <config.h>

#include "budg.h"
#include "item.h"
#include "land.h"
#include "lost.h"
#include "path.h"
#include "player.h"
#include "product.h"
#include "ship.h"
#include "update.h"

/*
 * Increase sector efficiency if old type == new type.
 * decrease sector efficiency if old type != new type.
 * Return amount of work used.
 */
static int
upd_buildeff(struct natstr *np, struct sctstr *sp, int *workp,
	     short *vec, int etu, int *desig, int sctwork, int *cost)
{
    int work_cost = 0;
    int buildeff_work = *workp / 2;
    int n, hcms, lcms, neweff;
    unsigned char old_type = *desig;

    *cost = 0;
    neweff = sp->sct_effic;

    if (*desig != sp->sct_newtype) {
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
	    *desig = sp->sct_newtype;
	}
	neweff = n;
	*cost += work_cost;
	if (!n && IS_BIG_CITY(old_type) &&
	    !IS_BIG_CITY(*desig)) {
	    int maxpop = max_population(np->nat_level[NAT_RLEV], *desig, n);
	    if (vec[I_CIVIL] > maxpop)
		vec[I_CIVIL] = maxpop;
	    if (vec[I_UW] > maxpop)
		vec[I_UW] = maxpop;
	    *workp = (vec[I_CIVIL] * sctwork) / 100.0
		+ (vec[I_MILIT] * 2 / 5.0) + vec[I_UW];
	    *workp = roundavg((etu * *workp) / 100.0);

	    buildeff_work = MIN((int)(*workp / 2), buildeff_work);
	}
    }
    if (*desig == sp->sct_newtype) {
	work_cost = 100 - neweff;
	if (work_cost > buildeff_work)
	    work_cost = buildeff_work;

	if (dchr[*desig].d_lcms > 0) {
	    lcms = vec[I_LCM];
	    lcms /= dchr[*desig].d_lcms;
	    if (work_cost > lcms)
		work_cost = lcms;
	}
	if (dchr[*desig].d_hcms > 0) {
	    hcms = vec[I_HCM];
	    hcms /= dchr[*desig].d_hcms;
	    if (work_cost > hcms)
		work_cost = hcms;
	}

	neweff += work_cost;
	*cost += work_cost * dchr[*desig].d_build;
	buildeff_work -= work_cost;

	if ((dchr[*desig].d_lcms > 0) || (dchr[*desig].d_hcms > 0)) {
	    vec[I_LCM] -= work_cost * dchr[*desig].d_lcms;
	    vec[I_HCM] -= work_cost * dchr[*desig].d_hcms;
	}
    }
    *workp = (*workp + 1) / 2 + buildeff_work;

    return neweff;
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

    /* Need to check treaties here */
    enlisted = 0;
    maxmil = (vec[I_CIVIL] / 2) - vec[I_MILIT];
    if (maxmil > 0) {
	enlisted = (etu * (10 + vec[I_MILIT]) * 0.05);
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
meltitems(int etus, int fallout, int own, short *vec, int type, int x, int y,
	  int uid)
{
    i_type n;
    int melt;

    for (n = I_NONE + 1; n <= I_MAX; n++) {
	melt = roundavg(vec[n] * etus * (long)fallout
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
 * Do fallout meltdown for sector SP.
 * ETUS above 24 are treated as 24 to avoid *huge* kill offs in
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
    meltitems(etus, sp->sct_fallout, sp->sct_own, sp->sct_item, EF_SECTOR,
	      sp->sct_x, sp->sct_y, 0);
    for (i = 0; NULL != (lp = getlandp(i)); i++) {
	if (!lp->lnd_own)
	    continue;
	if (lp->lnd_x != sp->sct_x || lp->lnd_y != sp->sct_y)
	    continue;
	meltitems(etus, sp->sct_fallout, lp->lnd_own, lp->lnd_item, EF_LAND,
		  lp->lnd_x, lp->lnd_y, lp->lnd_uid);
    }
    for (i = 0; NULL != (spp = getshipp(i)); i++) {
	if (!spp->shp_own)
	    continue;
	if (spp->shp_x != sp->sct_x || spp->shp_y != sp->sct_y)
	    continue;
	if (mchr[(int)spp->shp_type].m_flags & M_SUB)
	    continue;
	meltitems(etus, sp->sct_fallout, spp->shp_own, spp->shp_item, EF_SHIP,
		  spp->shp_x, spp->shp_y, spp->shp_uid);
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
produce_sect(int natnum, int etu, int *bp, long p_sect[][2])
{
    struct sctstr *sp;
    struct natstr *np;
    short buf[I_MAX + 1];
    short *vec;
    int work, cost, ecost, pcost, sctwork;
    int n, desig, maxpop, neweff, amount;

    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (sp->sct_type == SCT_WATER)
	    continue;
	if (sp->sct_own != natnum)
	    continue;
	if (sp->sct_updated != 0)
	    continue;

	np = getnatp(natnum);

	if ((sp->sct_type == SCT_CAPIT) && (sp->sct_effic > 60)) {
	    p_sect[SCT_CAPIT][0]++;
	    p_sect[SCT_CAPIT][1] += etu;
	    if (!player->simulation)
		np->nat_money -= etu;
	}

	if (player->simulation) {
	    /* work on a copy, which will be discarded */
	    memcpy(buf, sp->sct_item, sizeof(buf));
	    vec = buf;
	} else
	    vec = sp->sct_item;

	/* If everybody is dead, the sector reverts to unowned. 
	   * This is also checked at the end of the production in
	   * they all starved or were plagued off.
	 */
	if (vec[I_CIVIL] == 0 && vec[I_MILIT] == 0 &&
	    !has_units(sp->sct_x, sp->sct_y, sp->sct_own, 0)) {
	    makelost(EF_SECTOR, sp->sct_own, 0, sp->sct_x, sp->sct_y);
	    sp->sct_own = 0;
	    sp->sct_oldown = 0;
	    continue;
	}

	sp->sct_updated = 1;
	work = 0;

	/* do_feed trys to supply.  So, we need to enable cacheing
	   here */
	bp_enable_cachepath();

	sctwork = do_feed(sp, np, vec, &work, bp, etu);

	bp_disable_cachepath();
	bp_clear_cachepath();

	if (sp->sct_off || np->nat_money < 0)
	    continue;

	neweff = sp->sct_effic;
	amount = 0;
	pcost = cost = ecost = 0;

	desig = sp->sct_type;

	if ((sp->sct_effic < 100 || sp->sct_type != sp->sct_newtype) &&
	    np->nat_money > 0) {
	    neweff = upd_buildeff(np, sp, &work, vec, etu, &desig, sctwork,
				  &cost);
	    pt_bg_nmbr(bp, sp, I_LCM, vec[I_LCM]);
	    pt_bg_nmbr(bp, sp, I_HCM, vec[I_HCM]);
	    p_sect[SCT_EFFIC][0]++;
	    p_sect[SCT_EFFIC][1] += cost;
	    if (!player->simulation) {
		np->nat_money -= cost;
		sp->sct_type = desig;
		sp->sct_effic = neweff;
	    }
	}

	if (desig == SCT_ENLIST && neweff >= 60 &&
	    sp->sct_own == sp->sct_oldown) {
	    p_sect[desig][0] += enlist(vec, etu, &ecost);
	    p_sect[desig][1] += ecost;
	    if (!player->simulation)
		np->nat_money -= ecost;
	}

	/*
	 * now do the production (if sector effic >= 60%)
	 */

	if (neweff >= 60) {
	    if (np->nat_money > 0 && dchr[desig].d_prd >= 0)
		work -= produce(np, sp, vec, work, desig, neweff,
				&pcost, &amount);
	}

	pt_bg_nmbr(bp, sp, I_MAX + 1, work);
	p_sect[desig][0] += amount;
	p_sect[desig][1] += pcost;
	if (!player->simulation) {
	    maxpop = max_pop(np->nat_level[NAT_RLEV], sp);
	    if (vec[I_CIVIL] > maxpop)
		vec[I_CIVIL] = maxpop;
	    if (vec[I_UW] > maxpop)
		vec[I_UW] = maxpop;
	    sp->sct_avail = work;
	    np->nat_money -= pcost;
	}
    }
}
