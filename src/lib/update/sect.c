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
#include "item.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
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
	if (bp_skip_sect(bp, sp))
	    continue;
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
