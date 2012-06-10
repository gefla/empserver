/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  supply.c: Supply subroutines
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2004-2011
 */

#include <config.h>

#include <math.h>
#include "empobj.h"
#include "file.h"
#include "land.h"
#include "nat.h"
#include "optlist.h"
#include "path.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"

static int s_commod(struct empobj *, short *, i_type, int, int, int);
static int get_minimum(struct lndstr *, i_type);

int
sct_supply(struct sctstr *sp, i_type type, int wanted)
{
    return s_commod((struct empobj *)sp, sp->sct_item,
		    type, wanted, ITEM_MAX, 1);
}

int
shp_supply(struct shpstr *sp, i_type type, int wanted)
{
    return s_commod((struct empobj *)sp, sp->shp_item,
		    type, wanted, mchr[sp->shp_type].m_item[type], 1);
}

int
lnd_supply(struct lndstr *lp, i_type type, int wanted)
{
    return s_commod((struct empobj *)lp, lp->lnd_item,
		    type, wanted, lchr[lp->lnd_type].l_item[type], 1);
}

int
lnd_in_supply(struct lndstr *lp)
{
    if (!opt_NOFOOD) {
	if (lp->lnd_item[I_FOOD] < get_minimum(lp, I_FOOD))
	    return 0;
    }
    return lp->lnd_item[I_SHELL] >= get_minimum(lp, I_SHELL);
}

int
lnd_supply_all(struct lndstr *lp)
{
    int fail = 0;

    if (!opt_NOFOOD)
	fail |= !lnd_supply(lp, I_FOOD, get_minimum(lp, I_FOOD));
    fail |= !lnd_supply(lp, I_SHELL, get_minimum(lp, I_SHELL));
    return !fail;
}

/*
 * Actually get the commod
 *
 * First, try to forage in the sector
 * Second look for a warehouse or headquarters to leech
 * Third, look for a ship we own in a harbor
 * Fourth, look for supplies in a supply unit we own
 *		(one good reason to do this last is that the supply
 *		 unit will then call resupply, taking more time)
 *
 * May want to put code to resupply with SAMs here, later --ts
 */
static int
s_commod(struct empobj *sink, short *vec,
	 i_type type, int wanted, int limit, int actually_doit)
{
    natid own = sink->own;
    coord x = sink->x;
    coord y = sink->y;
    int lookrange;
    struct sctstr sect;
    struct nstr_sect ns;
    struct nstr_item ni;
    struct lchrstr *lcp;
    struct shpstr ship;
    struct lndstr land;
    /* leave at least 1 military in sectors/ships */
    int minimum = 0;
    int can_move;
    double move_cost, weight, mobcost;
    int packing;
    struct dchrstr *dp;
    struct ichrstr *ip;

    if (wanted > limit)
	wanted = limit;
    if (wanted <= vec[type])
	return 1;
    wanted -= vec[type];

    /* try to get it from sector we're in */
    if (sink->ef_type != EF_SECTOR) {
	getsect(x, y, &sect);
	if (sect.sct_own == own) {
	    if (!opt_NOFOOD && type == I_FOOD)
		minimum = 1 + (int)ceil(food_needed(sect.sct_item,
						    etu_per_update));
	    if (sect.sct_item[type] - wanted >= minimum) {
		sect.sct_item[type] -= wanted;
		if (actually_doit) {
		    vec[type] += wanted;
		    putsect(&sect);
		    put_empobj(sink->ef_type, sink->uid, sink);
		}
		return 1;
	    } else if (sect.sct_item[type] - minimum > 0) {
		wanted -= sect.sct_item[type] - minimum;
		sect.sct_item[type] = minimum;
		if (actually_doit) {
		    vec[type] += sect.sct_item[type] - minimum;
		    putsect(&sect);
		}
	    }
	}
    }

    /* look for a headquarters or warehouse */
    lookrange = tfact(own, 10.0);
    snxtsct_dist(&ns, x, y, lookrange);
    while (nxtsct(&ns, &sect) && wanted) {
	if (ns.curdist == 0)
	    continue;
	if (sect.sct_own != own)
	    continue;
	if ((sect.sct_type != SCT_WAREH) &&
	    (sect.sct_type != SCT_HEADQ) && (sect.sct_type != SCT_HARBR))
	    continue;
	if ((sect.sct_type == SCT_HEADQ) &&
	    (sect.sct_dist_x == sect.sct_x) &&
	    (sect.sct_dist_y == sect.sct_y))
	    continue;
	if (sect.sct_effic < 60)
	    continue;
	move_cost = path_find(sect.sct_x, sect.sct_y, x, y, own, MOB_MOVE);
	if (move_cost < 0)
	    continue;
	if (!opt_NOFOOD && type == I_FOOD)
	    minimum = 1 + (int)ceil(food_needed(sect.sct_item,
						etu_per_update));
	if (sect.sct_item[type] <= minimum)
	    continue;
	ip = &ichr[type];
	dp = &dchr[sect.sct_type];
	packing = ip->i_pkg[dp->d_pkg];
	if (packing > 1 && sect.sct_effic < 60)
	    packing = 1;
	weight = (double)ip->i_lbs / packing;
	mobcost = move_cost * weight;
	if (mobcost > 0)
	    can_move = (double)sect.sct_mobil / mobcost;
	else
	    can_move = sect.sct_item[type] - minimum;
	if (can_move > sect.sct_item[type] - minimum)
	    can_move = sect.sct_item[type] - minimum;

	if (can_move >= wanted) {
	    int n;

	    sect.sct_item[type] -= wanted;

	    /* take off mobility for delivering sect */
	    n = roundavg(wanted * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;
	    if (actually_doit) {
		vec[type] += wanted;
		putsect(&sect);
		put_empobj(sink->ef_type, sink->uid, sink);
	    }
	    return 1;
	} else if (can_move > 0) {
	    int n;
	    wanted -= can_move;
	    sect.sct_item[type] -= can_move;

	    /* take off mobility for delivering sect */
	    n = roundavg(can_move * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;
	    if (actually_doit) {
		vec[type] += can_move;
		putsect(&sect);
	    }
	}
    }

    /* look for an owned ship in a harbor */
    snxtitem_dist(&ni, EF_SHIP, x, y, lookrange);
    while (nxtitem(&ni, &ship) && wanted) {
	if (sink->ef_type == EF_SHIP && sink->uid == ship.shp_uid)
	    continue;
	if (ship.shp_own != own)
	    continue;
	if (!(mchr[(int)ship.shp_type].m_flags & M_SUPPLY))
	    continue;
	getsect(ship.shp_x, ship.shp_y, &sect);
	if (sect.sct_type != SCT_HARBR)
	    continue;
	if (sect.sct_effic < 2)
	    continue;
	move_cost = path_find(sect.sct_x, sect.sct_y, x, y, own, MOB_MOVE);
	if (move_cost < 0)
	    continue;
	if (!opt_NOFOOD && type == I_FOOD)
	    minimum = 1 + (int)ceil(food_needed(ship.shp_item,
						etu_per_update));
	if (ship.shp_item[type] <= minimum)
	    continue;
	ip = &ichr[type];
	dp = &dchr[sect.sct_type];
	packing = ip->i_pkg[dp->d_pkg];
	if (packing > 1 && sect.sct_effic < 60)
	    packing = 1;
	weight = (double)ip->i_lbs / packing;
	mobcost = move_cost * weight;
	if (mobcost > 0)
	    can_move = (double)sect.sct_mobil / mobcost;
	else
	    can_move = ship.shp_item[type] - minimum;
	if (can_move > ship.shp_item[type] - minimum)
	    can_move = ship.shp_item[type] - minimum;
	if (can_move >= wanted) {
	    int n;
	    ship.shp_item[type] -= wanted;

	    n = roundavg(wanted * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;
	    if (actually_doit) {
		vec[type] += can_move;
		putship(ship.shp_uid, &ship);
		putsect(&sect);
		put_empobj(sink->ef_type, sink->uid, sink);
	    }
	    return 1;
	} else if (can_move > 0) {
	    int n;
	    wanted -= can_move;
	    ship.shp_item[type] -= can_move;

	    n = roundavg(can_move * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;
	    if (actually_doit) {
		vec[type] += can_move;
		putship(ship.shp_uid, &ship);
		putsect(&sect);
	    }
	}
    }

    /* look for an owned supply unit */
    snxtitem_dist(&ni, EF_LAND, x, y, lookrange);
    while (nxtitem(&ni, &land) && wanted) {
	int min;

	if (sink->ef_type == EF_LAND && sink->uid == land.lnd_uid)
	    continue;
	if (land.lnd_own != own)
	    continue;

	lcp = &lchr[(int)land.lnd_type];
	if (!(lcp->l_flags & L_SUPPLY))
	    continue;

	if (land.lnd_item[type] <= get_minimum(&land, type))
	    continue;

	if (land.lnd_ship >= 0) {
	    getsect(land.lnd_x, land.lnd_y, &sect);
	    if (sect.sct_type != SCT_HARBR || sect.sct_effic < 2)
		continue;
	}

	move_cost = path_find(land.lnd_x, land.lnd_y, x, y, own, MOB_MOVE);
	if (move_cost < 0)
	    continue;

#if 0
	/*
	 * Recursive supply is disabled for now.  It can introduce
	 * cycles into the "resupplies from" relation.  The code below
	 * attempts to break these cycles by temporarily zapping the
	 * commodity being supplied.  That puts the land file in a
	 * funny state temporarily, risking loss of supplies when
	 * something goes wrong on the way.  Worse, it increases
	 * lnd_seqno even when !actually_doit, which can lead to
	 * spurious seqno mismatch oopses in users of
	 * lnd_could_be_supplied().  I can't be bothered to clean up
	 * this mess right now, because recursive resupply is too dumb
	 * to be really useful anyway: each step uses the first source
	 * it finds, without consideration of mobility cost.  If you
	 * re-enable it, don't forget to uncomment its documentation
	 * in supply.t as well.
	 */
	if (land.lnd_item[type] - wanted < get_minimum(&land, type)) {
	    struct lndstr save;

	    /*
	     * Temporarily zap this unit's store, so the recursion
	     * avoids it.
	     */
	    save = land;
	    land.lnd_item[type] = 0;
	    putland(land.lnd_uid, &land);
	    save.lnd_seqno = land.lnd_seqno;

	    s_commod((struct empobj *)&land, land.lnd_item, type, wanted,
		     lchr[land.lnd_type].l_item[type] - wanted,
		     actually_doit);
	    land.lnd_item[type] += save.lnd_item[type];

	    if (actually_doit)
		putland(land.lnd_uid, &land);
	    else
		putland(save.lnd_uid, &save);
	}
#endif

	min = get_minimum(&land, type);
	ip = &ichr[type];
	weight = ip->i_lbs;
	mobcost = move_cost * weight;
	if (mobcost > 0)
	    can_move = (double)land.lnd_mobil / mobcost;
	else
	    can_move = land.lnd_item[type] - min;
	if (can_move > land.lnd_item[type] - min)
	    can_move = land.lnd_item[type] - min;

	if (can_move >= wanted) {
	    land.lnd_item[type] -= wanted;
	    land.lnd_mobil -= roundavg(wanted * weight * move_cost);
	    if (actually_doit) {
		vec[type] += wanted;
		putland(land.lnd_uid, &land);
		put_empobj(sink->ef_type, sink->uid, sink);
	    }
	    return 1;
	} else if (can_move > 0) {
	    wanted -= can_move;
	    land.lnd_item[type] -= can_move;
	    land.lnd_mobil -= roundavg(can_move * weight * move_cost);
	    if (actually_doit) {
		vec[type] += can_move;
		putland(land.lnd_uid, &land);
	    }
	}
    }

    if (actually_doit)
	put_empobj(sink->ef_type, sink->uid, sink);
    return 0;
}

/*
 * We want to get enough shells to fire once,
 * one update's worth of food.
 */
static int
get_minimum(struct lndstr *lp, i_type type)
{
    struct lchrstr *lcp;
    int max, want = 0;

    lcp = &lchr[(int)lp->lnd_type];
    max = lcp->l_item[type];

    switch (type) {
    case I_FOOD:
	if (opt_NOFOOD)
	    return 0;		/* no food reqd, get out */
	want = (int)ceil(food_needed(lp->lnd_item, etu_per_update));
	break;
    case I_SHELL:
	want = lcp->l_ammo;
	break;
    default:
	return 0;
    }

    if (want > max)
	want = max;

    return want;
}

int
lnd_could_be_supplied(struct lndstr *lp)
{
    int res, food, food_needed, shells_needed, shells;

    if (!opt_NOFOOD) {
	food_needed = get_minimum(lp, I_FOOD);
	food = lp->lnd_item[I_FOOD];
	if (food < food_needed) {
	    res = s_commod((struct empobj *)lp, lp->lnd_item,
			   I_FOOD, food_needed,
			   lchr[lp->lnd_type].l_item[I_FOOD], 0);
	    lp->lnd_item[I_FOOD] = food;
	    if (!res)
		return 0;
	}
    }

    shells_needed = lchr[lp->lnd_type].l_ammo;
    shells = lp->lnd_item[I_SHELL];
    if (shells < shells_needed) {
	res = s_commod((struct empobj *)lp, lp->lnd_item,
		       I_SHELL, shells_needed,
		       lchr[lp->lnd_type].l_item[I_SHELL], 0);
	lp->lnd_item[I_SHELL] = shells;
	if (!res)
	    return 0;
    }

    return 1;
}
