/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  land.c: Do production for land units
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1992
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2009
 */

#include <config.h>

#include <math.h>
#include "budg.h"
#include "land.h"
#include "lost.h"
#include "news.h"
#include "plague.h"
#include "player.h"
#include "ship.h"
#include "update.h"

static void landrepair(struct lndstr *, struct natstr *, struct bp *, int);
static void upd_land(struct lndstr *, int, struct natstr *, struct bp *, int);
static int feed_land(struct lndstr *, int);

int
prod_land(int etus, int natnum, struct bp *bp, int build)
		/* build = 1, maintain = 0 */
{
    struct lndstr *lp;
    struct sctstr *sp;
    struct natstr *np;
    int n, k = 0;
    int start_money;

    for (n = 0; NULL != (lp = getlandp(n)); n++) {
	if (lp->lnd_own == 0)
	    continue;
	if (lp->lnd_own != natnum)
	    continue;
	if (lp->lnd_effic < LAND_MINEFF) {
	    makelost(EF_LAND, lp->lnd_own, lp->lnd_uid,
		     lp->lnd_x, lp->lnd_y);
	    lp->lnd_own = 0;
	    continue;
	}

	sp = getsectp(lp->lnd_x, lp->lnd_y);
	if (sp->sct_type == SCT_SANCT)
	    continue;
	np = getnatp(lp->lnd_own);
	start_money = np->nat_money;
	upd_land(lp, etus, np, bp, build);
	lnd_money[lp->lnd_own] += np->nat_money - start_money;
	if (!build || np->nat_money != start_money)
	    k++;
	if (player->simulation)
	    np->nat_money = start_money;
    }

    return k;
}

static void
upd_land(struct lndstr *lp, int etus,
	 struct natstr *np, struct bp *bp, int build)
	       /* build = 1, maintain = 0 */
{
    struct lchrstr *lcp;
    int pstage, ptime;
    int n;
    int min = morale_base - (int)np->nat_level[NAT_HLEV];
    int mult;
    int cost;
    int eff;

    if (!player->simulation)
	if (lp->lnd_retreat < min)
	    lp->lnd_retreat = min;

    lcp = &lchr[(int)lp->lnd_type];
    if (build == 1) {
	if (!lp->lnd_off && np->nat_money >= 0)
	    landrepair(lp, np, bp, etus);
	if (!player->simulation)
	    lp->lnd_off = 0;
    } else {
	mult = 1;
	if (np->nat_level[NAT_TLEV] < lp->lnd_tech * 0.85)
	    mult = 2;
	if (lcp->l_flags & L_ENGINEER)
	    mult *= 3;
	cost = -(mult * etus * MIN(0.0, money_land * lcp->l_cost));
	if (np->nat_money < cost && !player->simulation) {
	    if ((eff = lp->lnd_effic - etus / 5) < LAND_MINEFF) {
		wu(0, lp->lnd_own,
		   "%s lost to lack of maintenance\n", prland(lp));
		makelost(EF_LAND, lp->lnd_own, lp->lnd_uid,
			 lp->lnd_x, lp->lnd_y);
		lp->lnd_own = 0;
		lp->lnd_ship = lp->lnd_land = -1;
		return;
	    }
	    wu(0, lp->lnd_own,
	       "%s lost %d%% to lack of maintenance\n",
	       prland(lp), lp->lnd_effic - eff);
	    lp->lnd_effic = eff;
	} else {
	    np->nat_money -= cost;
	}

	if (!player->simulation) {
	    /* feed */
	    if ((n = feed_land(lp, etus)) > 0) {
		wu(0, lp->lnd_own, "%d starved in %s%s\n",
		   n, prland(lp),
		   (lp->lnd_effic < LAND_MINEFF ? ", killing it" : ""));
		if (n > 10)
		    nreport(lp->lnd_own, N_DIE_FAMINE, 0, 1);
	    }
	    /*
	     * do plague stuff.  plague can't break out on land units,
	     * but it can still kill people on them.
	     */
	    pstage = lp->lnd_pstage;
	    ptime = lp->lnd_ptime;
	    if (pstage != PLG_HEALTHY) {
		n = plague_people(np, lp->lnd_item, &pstage, &ptime, etus);
		switch (n) {
		case PLG_DYING:
		    wu(0, lp->lnd_own,
		       "PLAGUE deaths reported on %s\n", prland(lp));
		    nreport(lp->lnd_own, N_DIE_PLAGUE, 0, 1);
		    break;
		case PLG_INFECT:
		    wu(0, lp->lnd_own, "%s battling PLAGUE\n", prland(lp));
		    break;
		case PLG_INCUBATE:
		    /* Are we still incubating? */
		    if (n == pstage) {
			/* Yes. Will it turn "infectious" next time? */
			if (ptime <= etus) {
			    /* Yes.  Report an outbreak. */
			    wu(0, lp->lnd_own,
			       "Outbreak of PLAGUE on %s!\n", prland(lp));
			    nreport(lp->lnd_own, N_OUT_PLAGUE, 0, 1);
			}
		    } else {
			/* It has already moved on to "infectious" */
			wu(0, lp->lnd_own,
			   "%s battling PLAGUE\n", prland(lp));
		    }
		    break;
		case PLG_EXPOSED:
		    /* Has the plague moved to "incubation" yet? */
		    if (n != pstage) {
			/* Yes. Will it turn "infectious" next time? */
			if (ptime <= etus) {
			    /* Yes.  Report an outbreak. */
			    wu(0, lp->lnd_own,
			       "Outbreak of PLAGUE on %s!\n", prland(lp));
			    nreport(lp->lnd_own, N_OUT_PLAGUE, 0, 1);
			}
		    }
		    break;
		default:
		    break;
		}
		lp->lnd_pstage = pstage;
		lp->lnd_ptime = ptime;
	    }
	}			/* end !player->simulation */
    }
}

static void
landrepair(struct lndstr *land, struct natstr *np, struct bp *bp, int etus)
{
    int delta;
    struct sctstr *sp;
    struct lchrstr *lp;
    int build;
    int avail;
    int w_p_eff;
    int mult;
    int mvec[I_MAX + 1];

    lp = &lchr[(int)land->lnd_type];
    sp = getsectp(land->lnd_x, land->lnd_y);
    if (sp->sct_off)
	return;
    mult = 1;
    if (np->nat_level[NAT_TLEV] < land->lnd_tech * 0.85)
	mult = 2;

    if (land->lnd_effic == 100) {
	/* land is ok; no repairs needed */
	return;
    }
    if ((sp->sct_own != land->lnd_own) &&
	(getrel(getnatp(sp->sct_own), land->lnd_own) != ALLIED))
	return;

    if (!player->simulation)
	avail = sp->sct_avail * 100;
    else
	avail = bp_get_avail(bp, sp) * 100;

    w_p_eff = LND_BLD_WORK(lp->l_lcm, lp->l_hcm);
    delta = roundavg((double)avail / w_p_eff);
    if (delta <= 0)
	return;
    if (delta > (int)((float)etus * land_grow_scale))
	delta = (int)((float)etus * land_grow_scale);
    if (delta > 100 - land->lnd_effic)
	delta = 100 - land->lnd_effic;

    memset(mvec, 0, sizeof(mvec));
    mvec[I_LCM] = lp->l_lcm;
    mvec[I_HCM] = lp->l_hcm;
    build = get_materials(sp, bp, mvec, delta);

    if ((sp->sct_type != SCT_HEADQ) && (sp->sct_type != SCT_FORTR))
	build /= 3;

    avail -= build * w_p_eff;
    if (avail < 0)
	avail = 0;
    if (!player->simulation)
	sp->sct_avail = avail / 100;
    else
	bp_put_avail(bp, sp, avail / 100);

    if (build < 0)
	logerror("land unit %d building %d ! \n", land->lnd_uid, build);
    np->nat_money -= mult * lp->l_cost * build / 100.0;
    if (!player->simulation) {
	land->lnd_effic += (signed char)build;
    }
}

/*
 * returns the number who starved, if any.
 */
static int
feed_land(struct lndstr *lp, int etus)
{
    return feed_people(lp->lnd_item, etus);
}
