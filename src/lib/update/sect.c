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

#include "chance.h"
#include "file.h"
#include "item.h"
#include "land.h"
#include "lost.h"
#include "nat.h"
#include "optlist.h"
#include "path.h"
#include "player.h"
#include "prototypes.h"
#include "ship.h"
#include "update.h"

double
buildeff(struct sctstr *sp)
{
    int avail = sp->sct_avail / 2 * 100;
    double cost;
    int delta, build;
    struct dchrstr *dcp;

    cost = 0.0;

    if (sp->sct_type != sp->sct_newtype) {
	/*
	 * Tear down existing sector.
	 * Easier to destroy than to build.
	 */
	dcp = &dchr[sp->sct_type];
	build = 4 * avail / dcp->d_bwork;
	if (build <= sp->sct_effic)
	    sp->sct_effic -= build;
	else {
	    build = sp->sct_effic;
	    sp->sct_effic = 0;
	    sp->sct_type = sp->sct_newtype;
	}
	avail -= roundavg(build / 4.0 * dcp->d_bwork);
	cost += build / 4.0;
    }

    if (sp->sct_type == sp->sct_newtype) {
	dcp = &dchr[sp->sct_type];
	delta = avail / dcp->d_bwork;
	if (delta > 100 - sp->sct_effic)
	    delta = 100 - sp->sct_effic;
	build = get_materials(sp, dcp->d_mat, delta);
	sp->sct_effic += build;
	avail -= build * dcp->d_bwork;
	cost += build * dcp->d_cost / 100.0;
    }

    sp->sct_avail = (sp->sct_avail + 1) / 2 + avail / 100;
    return cost;
}

/*
 * enlistment sectors are special; they require military
 * to convert civ into mil in large numbers.
 * Conversion will happen much more slowly without
 * some mil initially.
 */
static void
enlist(struct natstr *np, short *vec, int etu)
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

    nat_budget[np->nat_cnum].prod[SCT_ENLIST].count += enlisted;
    nat_budget[np->nat_cnum].prod[SCT_ENLIST].money -= enlisted * 3;
    nat_budget[np->nat_cnum].money -= enlisted * 3;
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
produce_sect(int etu, struct bp *bp)
{
    struct budget *budget;
    struct natstr *np;
    struct sctstr *sp, scratch_sect;
    int n;
    double cost;

    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	if (sp->sct_type == SCT_WATER || sp->sct_type == SCT_SANCT)
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
	    bp_to_sect(bp, &scratch_sect);
	    sp = &scratch_sect;
	}

	budget = &nat_budget[sp->sct_own];
	np = getnatp(sp->sct_own);

	do_feed(sp, np, etu, 0);

	if (dchr[sp->sct_type].d_maint) {
	    cost = etu * dchr[sp->sct_type].d_maint;
	    budget->bm[BUDG_SCT_MAINT].count++;
	    budget->bm[BUDG_SCT_MAINT].money -= cost;
	    budget->money -= cost;
	}

	if (sp->sct_off || budget->money < 0) {
	    sp->sct_avail = 0;
	    bp_set_from_sect(bp, sp);
	    continue;
	}

	if ((sp->sct_effic < 100 || sp->sct_type != sp->sct_newtype) &&
	    budget->money >= 0) {
	    cost = buildeff(sp);
	    budget->bm[BUDG_SCT_BUILD].count++;
	    budget->bm[BUDG_SCT_BUILD].money -= cost;
	    budget->money -= cost;
	}

	if (sp->sct_type == SCT_ENLIST && sp->sct_effic >= 60 &&
	    sp->sct_own == sp->sct_oldown) {
	    enlist(np, sp->sct_item, etu);
	}

	/*
	 * now do the production (if sector effic >= 60%)
	 */

	if (sp->sct_effic >= 60) {
	    if (budget->money >= 0 && dchr[sp->sct_type].d_prd >= 0)
		produce(np, sp);
	}

	bp_set_from_sect(bp, sp);
    }
}
