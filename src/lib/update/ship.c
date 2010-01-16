/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ship.c: Do production for ships
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 *     Ron Koenderink, 2004
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
#include "product.h"
#include "ship.h"
#include "update.h"

static void shiprepair(struct shpstr *, struct natstr *, struct bp *, int);
static void upd_ship(struct shpstr *, int, struct natstr *, struct bp *, int);
static int feed_ship(struct shpstr *, int);

int
prod_ship(int etus, int natnum, struct bp *bp, int build)
		/* build = 1, maintain = 0 */
{
    struct shpstr *sp;
    struct natstr *np;
    int n, k = 0;
    int start_money;

    for (n = 0; NULL != (sp = getshipp(n)); n++) {
	if (sp->shp_own == 0)
	    continue;
	if (sp->shp_own != natnum)
	    continue;
	if (sp->shp_effic < SHIP_MINEFF) {
	    makelost(EF_SHIP, sp->shp_own, sp->shp_uid,
		     sp->shp_x, sp->shp_y);
	    sp->shp_own = 0;
	    continue;
	}

	np = getnatp(sp->shp_own);
	start_money = np->nat_money;
	upd_ship(sp, etus, np, bp, build);
	if (build && !player->simulation)	/* make sure to only autonav once */
	    nav_ship(sp);	/* autonav the ship */
	sea_money[sp->shp_own] += np->nat_money - start_money;
	if (!build || np->nat_money != start_money)
	    k++;
	if (player->simulation)
	    np->nat_money = start_money;
    }

    if (opt_SAIL) {
	if (build && !player->simulation)	/* make sure to only sail once */
	    sail_ship(natnum);
    }
    return k;
}

static void
upd_ship(struct shpstr *sp, int etus,
	 struct natstr *np, struct bp *bp, int build)
	       /* build = 1, maintain = 0 */
{
    struct sctstr *sectp;
    struct mchrstr *mp;
    int pstage, ptime;
    int oil_gained;
    int max_oil;
    int max_food;
    struct pchrstr *product;
    unsigned char *resource;
    int dep;
    int n;
    int mult;
    int cost;
    int eff;

    mp = &mchr[(int)sp->shp_type];
    if (build == 1) {
	if (!sp->shp_off && np->nat_money >= 0)
	    shiprepair(sp, np, bp, etus);
	if (!player->simulation)
	    sp->shp_off = 0;
    } else {
	mult = 1;
	if (np->nat_level[NAT_TLEV] < sp->shp_tech * 0.85)
	    mult = 2;
	cost = -(mult * etus * MIN(0.0, money_ship * mp->m_cost));
	if (np->nat_money < cost && !player->simulation) {
	    if ((eff = sp->shp_effic - etus / 5) < SHIP_MINEFF) {
		wu(0, sp->shp_own,
		   "%s lost to lack of maintenance\n", prship(sp));
		makelost(EF_SHIP, sp->shp_own, sp->shp_uid,
			 sp->shp_x, sp->shp_y);
		sp->shp_own = 0;
		return;
	    }
	    wu(0, sp->shp_own,
	       "%s lost %d%% to lack of maintenance\n",
	       prship(sp), sp->shp_effic - eff);
	    sp->shp_effic = eff;
	} else {
	    np->nat_money -= cost;
	}

	if (!player->simulation) {
	    sectp = getsectp(sp->shp_x, sp->shp_y);

	    /* produce oil */
	    if (np->nat_money >= 0
		&& (mp->m_flags & M_OIL) && sectp->sct_type == SCT_WATER) {
		product = &pchr[dchr[SCT_OIL].d_prd];
		oil_gained = roundavg(total_work(100, etus,
						 sp->shp_item[I_CIVIL],
					         sp->shp_item[I_MILIT],
						 sp->shp_item[I_UW],
						 ITEM_MAX)
				      * sp->shp_effic / 100.0
				      * sectp->sct_oil / 100.0
				      * prod_eff(SCT_OIL, sp->shp_tech));
		max_oil = mp->m_item[I_OIL];
		if (sp->shp_item[I_OIL] + oil_gained > max_oil)
		    oil_gained = max_oil - sp->shp_item[I_OIL];
		if (product->p_nrdep != 0 && oil_gained > 0) {
		    resource = (unsigned char *)sectp + product->p_nrndx;
		    if (*resource * 100 < product->p_nrdep * oil_gained)
			oil_gained = *resource * 100 / product->p_nrdep;
		    dep = roundavg(oil_gained * product->p_nrdep / 100.0);
		    if (CANT_HAPPEN(dep > *resource))
			dep = *resource;
		    *resource -= dep;
		}
		sp->shp_item[I_OIL] += oil_gained;
	    }
	    /* produce fish */
	    if (np->nat_money >= 0
		&& (mp->m_flags & M_FOOD) && sectp->sct_type == SCT_WATER) {
		sp->shp_item[I_FOOD]
		    += roundavg(total_work(100, etus,
					   sp->shp_item[I_CIVIL],
					   sp->shp_item[I_MILIT],
					   sp->shp_item[I_UW],
					   ITEM_MAX)
				* sp->shp_effic / 100.0
				* sectp->sct_fertil / 100.0
				* prod_eff(SCT_AGRI, sp->shp_tech));
	    }
	    /* feed */
	    if ((n = feed_ship(sp, etus)) > 0) {
		wu(0, sp->shp_own, "%d starved on %s\n", n, prship(sp));
		if (n > 10)
		    nreport(sp->shp_own, N_DIE_FAMINE, 0, 1);
	    }
	    max_food = mp->m_item[I_FOOD];
	    if (sp->shp_item[I_FOOD] > max_food)
		sp->shp_item[I_FOOD] = max_food;
	    /*
	     * do plague stuff.  plague can't break out on ships,
	     * but it can still kill people.
	     */
	    pstage = sp->shp_pstage;
	    ptime = sp->shp_ptime;
	    if (pstage != PLG_HEALTHY) {
		n = plague_people(np, sp->shp_item, &pstage, &ptime, etus);
		switch (n) {
		case PLG_DYING:
		    wu(0, sp->shp_own,
		       "PLAGUE deaths reported on %s\n", prship(sp));
		    nreport(sp->shp_own, N_DIE_PLAGUE, 0, 1);
		    break;
		case PLG_INFECT:
		    wu(0, sp->shp_own, "%s battling PLAGUE\n", prship(sp));
		    break;
		case PLG_INCUBATE:
		    /* Are we still incubating? */
		    if (n == pstage) {
			/* Yes. Will it turn "infectious" next time? */
			if (ptime <= etus) {
			    /* Yes.  Report an outbreak. */
			    wu(0, sp->shp_own,
			       "Outbreak of PLAGUE on %s!\n", prship(sp));
			    nreport(sp->shp_own, N_OUT_PLAGUE, 0, 1);
			}
		    } else {
			/* It has already moved on to "infectious" */
			wu(0, sp->shp_own,
			   "%s battling PLAGUE\n", prship(sp));
		    }
		    break;
		case PLG_EXPOSED:
		    /* Has the plague moved to "incubation" yet? */
		    if (n != pstage) {
			/* Yes. Will it turn "infectious" next time? */
			if (ptime <= etus) {
			    /* Yes.  Report an outbreak. */
			    wu(0, sp->shp_own,
			       "Outbreak of PLAGUE on %s!\n", prship(sp));
			    nreport(sp->shp_own, N_OUT_PLAGUE, 0, 1);
			}
		    }
		    break;
		default:
		    break;
		}

		sp->shp_pstage = pstage;
		sp->shp_ptime = ptime;
	    }
	    pops[sp->shp_own] += sp->shp_item[I_CIVIL];
	}
    }
}

/*
 * idea is: a sector full of workers can fix up eight
 * battleships +8 % eff each etu.  This will cost around
 * 8 * 8 * $40 = $2560!
 */
static void
shiprepair(struct shpstr *ship, struct natstr *np, struct bp *bp, int etus)
{
    int delta;
    struct sctstr *sp;
    struct mchrstr *mp;
    int build;
    int wf;
    int avail;
    int w_p_eff;
    int mult;
    int mvec[I_MAX + 1];
    int rel;

    mp = &mchr[(int)ship->shp_type];
    sp = getsectp(ship->shp_x, ship->shp_y);

    if ((sp->sct_own != ship->shp_own) && (sp->sct_own != 0)) {
	rel = getrel(getnatp(sp->sct_own), ship->shp_own);

	if (rel < FRIENDLY)
	    return;
    }

    /* only military can work on a military boat */
    if (mp->m_glim != 0)
	wf = etus * ship->shp_item[I_MILIT] / 2;
    else
	wf = etus * (ship->shp_item[I_CIVIL] / 2 + ship->shp_item[I_MILIT] / 5);

    if (sp->sct_type != SCT_HARBR) {
	wf /= 3;
	avail = wf;
    } else {
	if (!player->simulation)
	    avail = wf + sp->sct_avail * 100;
	else
	    avail = wf + bp_get_avail(bp, sp) * 100;
    }

    w_p_eff = SHP_BLD_WORK(mp->m_lcm, mp->m_hcm);

    if ((sp->sct_off) && (sp->sct_own == ship->shp_own))
	return;

    mult = 1;
    if (np->nat_level[NAT_TLEV] < ship->shp_tech * 0.85)
	mult = 2;

    if (ship->shp_effic == 100) {
	/* ship is ok; no repairs needed */
	return;
    }

    delta = roundavg((double)avail / w_p_eff);
    if (delta <= 0)
	return;
    if (delta > (int)((float)etus * ship_grow_scale))
	delta = (int)((float)etus * ship_grow_scale);
    if (delta > 100 - ship->shp_effic)
	delta = 100 - ship->shp_effic;

    memset(mvec, 0, sizeof(mvec));
    mvec[I_LCM] = mp->m_lcm;
    mvec[I_HCM] = mp->m_hcm;
    build = get_materials(sp, bp, mvec, delta);

    if (sp->sct_type != SCT_HARBR)
	build = delta;

    wf -= build * w_p_eff;
    if (wf < 0) {
	/*
	 * I didn't use roundavg here, because I want to penalize
	 * the player with a large number of ships.
	 */
	if (!player->simulation)
	    avail = (sp->sct_avail * 100 + wf) / 100;
	else
	    avail = (bp_get_avail(bp, sp) * 100 + wf) / 100;
	if (avail < 0)
	    avail = 0;
	if (!player->simulation)
	    sp->sct_avail = avail;
	else
	    bp_put_avail(bp, sp, avail);
    }
    if (sp->sct_type != SCT_HARBR)
	if ((build + ship->shp_effic) > 80) {
	    build = 80 - ship->shp_effic;
	    if (build < 0)
		build = 0;
	}

    np->nat_money -= mult * mp->m_cost * build / 100.0;
    if (!player->simulation)
	ship->shp_effic += (signed char)build;
}

/*
 * returns the number who starved, if any.
 */
static int
feed_ship(struct shpstr *sp, int etus)
{
    int needed, take;
    double give;
    struct nstr_item ni;
    struct lndstr *lp;

    if (opt_NOFOOD)
	return 0;

    needed = (int)ceil(food_needed(sp->shp_item, etus));

    /* scrounge */
    if (needed > sp->shp_item[I_FOOD]) {
	/* take from embarked land units, but don't starve them */
	snxtitem_cargo(&ni, EF_LAND, EF_SHIP, sp->shp_uid);
	while ((lp = nxtitemp(&ni)) && needed > sp->shp_item[I_FOOD]) {
	    give = lp->lnd_item[I_FOOD] - food_needed(lp->lnd_item, etus);
	    if (give < 1.0)
		continue;
	    take = MIN((int)give, needed - sp->shp_item[I_FOOD]);
	    sp->shp_item[I_FOOD] += take;
	    lp->lnd_item[I_FOOD] -= take;
	}
    }

    return feed_people(sp->shp_item, etus);
}
