/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  ship.c: Do production for ships
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1996
 */

#include "misc.h"
#include "var.h"
#include "sect.h"
#include "nat.h"
#include "ship.h"
#include "var.h"
#include "news.h"
#include "file.h"
#include "product.h"
#include "land.h"
#include "xy.h"
#include "nsc.h"
#include "optlist.h"
#include "player.h"
#include "update.h"
#include "common.h"
#include "subs.h"
#include "gen.h"
#include "lost.h"
#include "budg.h"

#ifndef MIN
#define MIN(x,y)        ((x) > (y) ? (y) : (x))
#endif

static int shiprepair(register struct shpstr *, int *, struct natstr *,
		      int *, int);
static void upd_ship(register struct shpstr *, register int,
		     struct natstr *, int *, int);

int
prod_ship(int etus, int natnum, int *bp, int build)
		/* build = 1, maintain = 0 */
{
    register struct shpstr *sp;
    struct natstr *np;
    int n, k = 0;
    int start_money;
    int lastx = 9999, lasty = 9999;

    bp_enable_cachepath();
    for (n = 0; NULL != (sp = getshipp(n)); n++) {
	if (sp->shp_own == 0)
	    continue;
	if (sp->shp_own != natnum)
	    continue;
	np = getnatp(sp->shp_own);
	start_money = np->nat_money;
	if (lastx == 9999 || lasty == 9999) {
	    lastx = sp->shp_x;
	    lasty = sp->shp_y;
	}
	if (lastx != sp->shp_x || lasty != sp->shp_y) {
	    /* Reset the cache */
	    bp_disable_cachepath();
	    bp_clear_cachepath();
	    bp_enable_cachepath();
	}
	upd_ship(sp, etus, np, bp, build);
	if (build && !player->simulation)	/* make sure to only autonav once */
	    nav_ship(sp);	/* autonav the ship */
	sea_money[sp->shp_own] += np->nat_money - start_money;
	if ((build && (np->nat_money != start_money)) || (!build))
	    k++;
	if (player->simulation)
	    np->nat_money = start_money;
    }
    bp_disable_cachepath();
    bp_clear_cachepath();

    if (opt_SAIL) {
	if (build && !player->simulation)	/* make sure to only sail once */
	    sail_ship(natnum);
    }
    return k;
}

static void
upd_ship(register struct shpstr *sp, register int etus,
	 struct natstr *np, int *bp, int build)
	       /* build = 1, maintain = 0 */
{
    struct sctstr *sectp;
    struct mchrstr *mp;
    int vec[I_MAX + 1];
    int cvec[I_MAX + 1];
    int oil_gained;
    int max_oil;
    int max_food;
    struct pchrstr *product;
    s_char *resource;
    int n;
    int mult;
    int needed;
    int cost;
    int eff;

    mp = &mchr[(int)sp->shp_type];
    getvec(VT_ITEM, vec, (s_char *)sp, EF_SHIP);
    if (build == 1) {
	if (np->nat_priorities[PRI_SBUILD] == 0 || np->nat_money < 0)
	    return;
	if (sp->shp_effic < SHIP_MINEFF ||
	    !shiprepair(sp, vec, np, bp, etus)) {
	    makelost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x,
		     sp->shp_y);
	    sp->shp_own = 0;
	    return;
	}
    } else {
	mult = 1;
	if (np->nat_level[NAT_TLEV] < sp->shp_tech * 0.85)
	    mult = 2;
	cost = -(mult * etus * dmin(0.0, money_ship * mp->m_cost));
	if ((np->nat_priorities[PRI_SMAINT] == 0 ||
	     np->nat_money < cost) && !player->simulation) {
	    if ((eff = sp->shp_effic - etus / 5) < SHIP_MINEFF) {
		wu(0, sp->shp_own,
		   "%s lost to lack of maintenance\n", prship(sp));
		makelost(EF_SHIP, sp->shp_own, sp->shp_uid, sp->shp_x,
			 sp->shp_y);
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

	sectp = getsectp(sp->shp_x, sp->shp_y);
	if (((mp->m_flags & M_OIL) && (sectp->sct_type == SCT_WATER))
	    && !player->simulation) {
	    /*
	     * take care of oil production
	     */
	    oil_gained = roundavg((vec[I_CIVIL] * etus / 10000.0)
				  * sectp->sct_oil);
	    vec[I_OIL] += oil_gained;
	    max_oil = vl_find(V_OIL, mp->m_vtype, mp->m_vamt, mp->m_nv);
	    if (vec[I_OIL] > max_oil)
		vec[I_OIL] = max_oil;
	    product = &pchr[P_OIL];
	    if (product->p_nrdep != 0 && oil_gained > 0) {
		resource = ((s_char *)sectp) + product->p_nrndx;
		*resource -= roundavg(oil_gained *
				      product->p_nrdep / 100.0);
	    }
	}
	if (((mp->m_flags & M_FOOD) && (sectp->sct_type == SCT_WATER))
	    && !player->simulation) {
	    sectp = getsectp(sp->shp_x, sp->shp_y);
	    vec[I_FOOD] += ((vec[I_CIVIL] * etus) / 1000.0)
		* sectp->sct_fertil;
	}
/* Military costs are now part of regular military costs, not ship costs */
/*		np->nat_money += (int) (etus * vec[I_MILIT] * money_mil);*/
	if (!player->simulation) {
	    if ((n = feed_ship(sp, vec, etus, &needed, 1)) > 0) {
		wu(0, sp->shp_own, "%d starved on %s\n", n, prship(sp));
		if (n > 10)
		    nreport(sp->shp_own, N_DIE_FAMINE, 0, 1);
	    }
	    max_food = vl_find(V_FOOD, mp->m_vtype, mp->m_vamt, mp->m_nv);
	    if (vec[I_FOOD] > max_food)
		vec[I_FOOD] = max_food;
	    /*
	     * do plague stuff.  plague can't break out on ships,
	     * but it can still kill people.
	     */
	    getvec(VT_COND, cvec, (s_char *)sp, EF_SHIP);
	    if (cvec[C_PSTAGE] > 0) {
		n = plague_people(np, vec, cvec, etus);
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
		    if (n == cvec[C_PSTAGE]) {
			/* Yes. Will it turn "infectious" next time? */
			if (cvec[C_PTIME] <= etus) {
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
		    if (n != cvec[C_PSTAGE]) {
			/* Yes. Will it turn "infectious" next time? */
			if (cvec[C_PTIME] <= etus) {
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

		putvec(VT_COND, cvec, (s_char *)sp, EF_SHIP);
	    }
	    putvec(VT_ITEM, vec, (s_char *)sp, EF_SHIP);
	    pops[sp->shp_own] += vec[I_CIVIL];
	}
    }
}

/*
 * idea is: a sector full of workers can fix up eight
 * battleships +8 % eff each etu.  This will cost around
 * 8 * 8 * $40 = $2560!
 */
static int
shiprepair(register struct shpstr *ship, int *vec, struct natstr *np,
	   int *bp, int etus)
{
    register int delta;
    struct sctstr *sp;
    struct mchrstr *mp;
    float leftp, buildp;
    int left, build;
    int lcm_needed, hcm_needed;
    int wf;
    int avail;
    int w_p_eff;
    int mult;
    int svec[I_MAX + 1];
    int mvec[I_MAX + 1];
    int rel;

    mp = &mchr[(int)ship->shp_type];
    sp = getsectp(ship->shp_x, ship->shp_y);

    if ((sp->sct_own != ship->shp_own) && (sp->sct_own != 0)) {
	rel = getrel(getnatp(sp->sct_own), ship->shp_own);

	if (rel < FRIENDLY)
	    return 1;
    }

    wf = 0;
    /* only military can work on a military boat */
    if (ship->shp_glim > 0)
	wf = etus * vec[I_MILIT] / 2;
    else
	wf = etus * (vec[I_CIVIL] / 2 + vec[I_MILIT] / 5);

    if (sp->sct_type != SCT_HARBR) {
	wf /= 3;
	avail = wf;
    } else {
	if (!player->simulation)
	    avail = wf + sp->sct_avail * 100;
	else
	    avail = wf + gt_bg_nmbr(bp, sp, I_MAX + 1) * 100;
    }

    w_p_eff = 20 + (mp->m_lcm + 2 * mp->m_hcm);

    if (sp->sct_type != SCT_HARBR) {
	int abs_max, amt;

	if (ship->shp_glim > 0) {
	    abs_max = vl_find(V_MILIT, mp->m_vtype,
			      mp->m_vamt, (int)mp->m_nv);
	    amt = vec[I_MILIT];
	} else {
	    abs_max = vl_find(V_CIVIL, mp->m_vtype,
			      mp->m_vamt, (int)mp->m_nv);
	    amt = vec[I_CIVIL];
	    if (abs_max == 0) {
		abs_max = vl_find(V_MILIT, mp->m_vtype, mp->m_vamt,
				  (int)mp->m_nv);
		amt = vec[I_MILIT];
	    }
	}

	if (abs_max == 0) {
	    logerror("Abs max of 0 for ship %d\n", ship->shp_uid);
	    abs_max = 1;
	}
	avail -= (etus * (100 - ((amt * 100) / abs_max))) / 7;
	/* think of it as entropy in action */
    }

    if (avail <= 0) {
	if (!player->simulation) {
	    if (opt_SHIP_DECAY) {
		ship->shp_effic += avail / w_p_eff;
	    }
	    return 1;
	}
    }

    if ((sp->sct_off) && (sp->sct_own == ship->shp_own))
	return 1;

    getvec(VT_ITEM, svec, (s_char *)sp, EF_SECTOR);

    mult = 1;
    if (np->nat_level[NAT_TLEV] < ship->shp_tech * 0.85)
	mult = 2;

    if (ship->shp_effic == 100) {
	/* ship is ok; no repairs needed */
	return 1;
    }

    left = 100 - ship->shp_effic;
    delta = roundavg((double)avail / w_p_eff);
    if (delta <= 0)
	return 1;
    if (delta > etus * ship_grow_scale)
	delta = etus * ship_grow_scale;
    if (delta > left)
	delta = left;

    /* delta is the max amount we can grow */

    left = 100 - ship->shp_effic;
    if (left > delta)
	left = delta;

    leftp = ((float)left / 100.0);
    memset(mvec, 0, sizeof(mvec));
    mvec[I_LCM] = lcm_needed = ldround((double)(mp->m_lcm * leftp), 1);
    mvec[I_HCM] = hcm_needed = ldround((double)(mp->m_hcm * leftp), 1);

    get_materials(sp, bp, mvec, 0);

    if (mvec[I_LCM] >= lcm_needed)
	buildp = leftp;
    else
	buildp = ((float)mvec[I_LCM] / (float)mp->m_lcm);
    if (mvec[I_HCM] < hcm_needed)
	buildp = MIN(buildp, ((float)mvec[I_HCM] / (float)mp->m_hcm));

    build = ldround((double)(buildp * 100.0), 1);
    memset(mvec, 0, sizeof(mvec));
    mvec[I_LCM] = lcm_needed = roundavg((double)(mp->m_lcm * buildp));
    mvec[I_HCM] = hcm_needed = roundavg((double)(mp->m_hcm * buildp));

    get_materials(sp, bp, mvec, 1);

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
	    avail = (gt_bg_nmbr(bp, sp, I_MAX + 1) * 100 + wf) / 100;
	if (avail < 0)
	    avail = 0;
	if (!player->simulation)
	    sp->sct_avail = avail;
	else
	    pt_bg_nmbr(bp, sp, I_MAX + 1, avail);
    }
    if (sp->sct_type != SCT_HARBR)
	if ((build + ship->shp_effic) > 80) {
	    build = 80 - ship->shp_effic;
	    if (build < 0)
		build = 0;
	}

    np->nat_money -= mult * mp->m_cost * build / 100.0;
    if (!player->simulation)
	ship->shp_effic += (s_char)build;
    return 1;
}

/*
 * returns the number who starved, if any.
 */
int
feed_ship(struct shpstr *sp, register int *vec, int etus, int *needed,
	  int doit)
{
    double food_eaten, land_eaten;
    double people_left;
    int ifood_eaten;
    int can_eat, need;
    int total_people;
    int to_starve;
    int starved, lvec[I_MAX + 1];
    struct nstr_item ni;
    struct lndstr *lp;

    if (opt_NOFOOD)
	return 0;		/* no food no work to do */

    food_eaten =
	(etus * eatrate) * (vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW]);
    ifood_eaten = (int)food_eaten;
    if ((food_eaten - ifood_eaten) > 0)
	ifood_eaten++;
    starved = 0;
    *needed = 0;
    if (!player->simulation && food_eaten > vec[I_FOOD])
	vec[I_FOOD] += supply_commod(sp->shp_own, sp->shp_x, sp->shp_y,
				     I_FOOD, (ifood_eaten - vec[I_FOOD]));

    if (food_eaten > vec[I_FOOD]) {
/* doit - only steal food from land units during the update */
	if (sp->shp_nland > 0 && doit) {
	    snxtitem_all(&ni, EF_LAND);
	    while ((lp = (struct lndstr *)nxtitemp(&ni, 0)) &&
		   (food_eaten > vec[I_FOOD])) {
		if (lp->lnd_ship != sp->shp_uid)
		    continue;
		need = ifood_eaten - vec[I_FOOD];
		getvec(VT_ITEM, lvec, (s_char *)lp, EF_LAND);
		land_eaten = (etus * eatrate) * (double)lnd_getmil(lp);
		if (lvec[I_FOOD] - need > land_eaten) {
		    vec[I_FOOD] += need;
		    lvec[I_FOOD] -= need;
		} else if ((lvec[I_FOOD] - land_eaten) > 0) {
		    vec[I_FOOD] += (lvec[I_FOOD] - land_eaten);
		    lvec[I_FOOD] -= (lvec[I_FOOD] - land_eaten);
		}
		putvec(VT_ITEM, lvec, (s_char *)lp, EF_LAND);
	    }
	}
    }

    if (food_eaten > vec[I_FOOD]) {
	*needed = food_eaten - vec[I_FOOD];
	if (*needed < (food_eaten - vec[I_FOOD]))
	    (*needed)++;
	if (opt_NEW_STARVE) {
	    can_eat = (vec[I_FOOD] / (etus * eatrate));
	    total_people = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];

	    /* only want to starve off at most 1/2 the populace. */
	    if (can_eat < (total_people / 2))
		can_eat = total_people / 2;

	    to_starve = total_people - can_eat;
	    while (to_starve && vec[I_UW]) {
		to_starve--;
		starved++;
		vec[I_UW]--;
	    }
	    while (to_starve && vec[I_CIVIL]) {
		to_starve--;
		starved++;
		vec[I_CIVIL]--;
	    }
	    while (to_starve && vec[I_MILIT]) {
		to_starve--;
		starved++;
		vec[I_MILIT]--;
	    }

	    vec[I_FOOD] = 0;
	} else {		/* ! opt_NEW_STARVE */
	    people_left = (vec[I_FOOD] + 0.01) / (food_eaten + 0.01);
	    starved = vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
	    /* only want to starve off at most 1/2 the populace. */
	    if (people_left < 0.5)
		people_left = 0.5;
	    vec[I_CIVIL] = (int)(vec[I_CIVIL] * people_left);
	    vec[I_MILIT] = (int)(vec[I_MILIT] * people_left);
	    vec[I_UW] = (int)(vec[I_UW] * people_left);
	    starved -= vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW];
	    vec[I_FOOD] = 0;
	}
    } else {
	vec[I_FOOD] -= (int)food_eaten;
    }
    return starved;
}
