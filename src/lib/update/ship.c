/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  ship.c: Do production for ships
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 *     Ron Koenderink, 2004
 *     Markus Armbruster, 2006-2016
 */

#include <config.h>

#include <math.h>
#include "chance.h"
#include "land.h"
#include "lost.h"
#include "nat.h"
#include "news.h"
#include "optlist.h"
#include "plague.h"
#include "player.h"
#include "product.h"
#include "prototypes.h"
#include "ship.h"
#include "update.h"

static void upd_ship(struct shpstr *, int, struct bp *, int);
static void plague_ship(struct shpstr *, int);
static void shiprepair(struct shpstr *, struct natstr *, struct bp *,
		       int, struct budget *);
static void ship_produce(struct shpstr *, int, struct budget *);
static int feed_ship(struct shpstr *, int);

void prep_ships(int etus, struct bp *bp)
{
    int mil, i, n;
    double mil_pay;
    struct shpstr *sp;

    for (i = 0; (sp = getshipp(i)); i++) {
	if (sp->shp_own == 0)
	    continue;
	if (CANT_HAPPEN(sp->shp_effic < SHIP_MINEFF)) {
	    makelost(EF_SHIP, sp->shp_own, sp->shp_uid,
		     sp->shp_x, sp->shp_y);
	    sp->shp_own = 0;
	    continue;
	}

	bp_consider_unit(bp, (struct empobj *)sp);
	mil = sp->shp_item[I_MILIT];
	mil_pay = mil * etus * money_mil;
	nat_budget[sp->shp_own].mil.count += mil;
	nat_budget[sp->shp_own].mil.money += mil_pay;
	nat_budget[sp->shp_own].money += mil_pay;

	if (!player->simulation) {
	    if ((n = feed_ship(sp, etus)) > 0) {
		wu(0, sp->shp_own, "%d starved on %s\n", n, prship(sp));
		if (n > 10)
		    nreport(sp->shp_own, N_DIE_FAMINE, 0, 1);
	    }
	    plague_ship(sp, etus);
	}
    }
}

void
prod_ship(int etus, struct bp *bp, int build)
		/* build = 1, maintain = 0 */
{
    struct shpstr *sp;
    int i;

    for (i = 0; (sp = getshipp(i)); i++) {
	if (sp->shp_own == 0)
	    continue;
	if (bp_skip_unit(bp, (struct empobj *)sp))
	    continue;
	upd_ship(sp, etus, bp, build);
    }
}

static void
upd_ship(struct shpstr *sp, int etus, struct bp *bp, int build)
	       /* build = 1, maintain = 0 */
{
    struct budget *budget = &nat_budget[sp->shp_own];
    struct mchrstr *mp = &mchr[sp->shp_type];
    struct natstr *np = getnatp(sp->shp_own);
    int mult, eff_lost;
    double cost;

    if (build == 1) {
	if (!sp->shp_off && budget->money >= 0)
	    shiprepair(sp, np, bp, etus, budget);
	ship_produce(sp, etus, budget);
	if (!player->simulation)
	    sp->shp_off = 0;
    } else {
	budget->oldowned_civs += sp->shp_item[I_CIVIL];
	mult = 1;
	if (np->nat_level[NAT_TLEV] < sp->shp_tech * 0.85)
	    mult = 2;
	budget->bm[BUDG_SHP_MAINT].count++;
	cost = mult * etus * -money_ship * mp->m_cost;
	if (budget->money < cost && !player->simulation) {
	    eff_lost = etus / 5;
	    if (sp->shp_effic - eff_lost < SHIP_MINEFF)
		eff_lost = sp->shp_effic - SHIP_MINEFF;
	    if (eff_lost > 0) {
		wu(0, sp->shp_own, "%s lost %d%% to lack of maintenance\n",
		   prship(sp), eff_lost);
		sp->shp_effic -= eff_lost;
	    }
	} else {
	    budget->bm[BUDG_SHP_MAINT].money -= cost;
	    budget->money -= cost;
	}
    }
}

static void
plague_ship(struct shpstr *sp, int etus)
{
    struct natstr *np = getnatp(sp->shp_own);
    int pstage, ptime;
    int n;

    /* Plague can't break out on ships, but it can still kill people */
    pstage = sp->shp_pstage;
    ptime = sp->shp_ptime;
    if (pstage != PLG_HEALTHY) {
	n = plague_people(np, sp->shp_item, &pstage, &ptime, etus);
	if (n != PLG_HEALTHY)
	    plague_report(sp->shp_own, n, pstage, ptime, etus,
			  "on", prship(sp));
	sp->shp_pstage = pstage;
	sp->shp_ptime = ptime;
    }
}

/*
 * idea is: a sector full of workers can fix up eight
 * battleships +8 % eff each etu.  This will cost around
 * 8 * 8 * $40 = $2560!
 */
static void
shiprepair(struct shpstr *ship, struct natstr *np, struct bp *bp, int etus,
	   struct budget *budget)
{
    struct mchrstr *mp = &mchr[(int)ship->shp_type];
    int delta;
    struct sctstr *sp, scratch_sect;
    int build;
    int wf;
    int avail;
    int mult;
    double cost;

    if (ship->shp_effic == 100)
	return;

    sp = getsectp(ship->shp_x, ship->shp_y);
    if (sp->sct_off)
	return;

    if (sp->sct_own != 0
	&& relations_with(sp->sct_own, ship->shp_own) < FRIENDLY)
	return;

    if (player->simulation) {
	scratch_sect = *sp;
	bp_to_sect(bp, &scratch_sect);
	sp = &scratch_sect;
    }

    mult = 1;
    if (np->nat_level[NAT_TLEV] < ship->shp_tech * 0.85)
	mult = 2;

    /* only military can work on a military boat */
    if (mp->m_glim != 0)
	wf = etus * ship->shp_item[I_MILIT] / 2;
    else
	wf = etus * (ship->shp_item[I_CIVIL] / 2 + ship->shp_item[I_MILIT] / 5);

    if (sp->sct_type != SCT_HARBR) {
	wf /= 3;
	avail = wf;
    } else
	avail = wf + sp->sct_avail * 100;

    delta = avail / mp->m_bwork;
    if (delta <= 0)
	return;
    if (delta > (int)((float)etus * ship_grow_scale))
	delta = (int)((float)etus * ship_grow_scale);
    if (delta > 100 - ship->shp_effic)
	delta = 100 - ship->shp_effic;

    build = get_materials(sp, mp->m_mat, delta);

    if (sp->sct_type != SCT_HARBR)
	build = delta;

    wf -= build * mp->m_bwork;
    if (wf < 0) {
	avail = roundavg((sp->sct_avail * 100 + wf) / 100.0);
	if (avail < 0)
	    avail = 0;
	sp->sct_avail = avail;
    }
    if (sp->sct_type != SCT_HARBR)
	if ((build + ship->shp_effic) > 80) {
	    build = 80 - ship->shp_effic;
	    if (build < 0)
		build = 0;
	}

    bp_set_from_sect(bp, sp);
    cost = mult * mp->m_cost * build / 100.0;
    budget->bm[BUDG_SHP_BUILD].count += !!build;
    budget->bm[BUDG_SHP_BUILD].money -= cost;
    budget->money -= cost;
    if (!player->simulation)
	ship->shp_effic += (signed char)build;
}

static void
ship_produce(struct shpstr *sp, int etus, struct budget *budget)
{
    struct mchrstr *mp = &mchr[sp->shp_type];
    struct sctstr *sectp = getsectp(sp->shp_x, sp->shp_y);
    int oil_gained;
    int max_oil;
    int max_food;
    struct pchrstr *product;
    unsigned char *resource;
    int dep;

    if (player->simulation)
	return;

    /* produce oil */
    if (!sp->shp_off && budget->money >= 0
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
    if (!sp->shp_off && budget->money >= 0
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
    max_food = mp->m_item[I_FOOD];
    if (sp->shp_item[I_FOOD] > max_food)
	sp->shp_item[I_FOOD] = max_food;
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
