/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  supply.c: Supply subroutines
 * 
 *  Known contributors to this file:
 *  
 */

#include <config.h>

#include <math.h>
#include "file.h"
#include "land.h"
#include "nat.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"

static int get_minimum(struct lndstr *, i_type);
static int s_commod(int, int, int, i_type, int, int);

/*
 * We want to get enough guns to be maxed out, enough shells to
 *	fire once, one update's worth of food.
 *
 * Firts, try to forage in the sector
 * Second look for a warehouse or headquarters to leech
 * Third, look for a ship we own in a harbor
 * Fourth, look for supplies in a supply unit we own
 *		(one good reason to do this last is that the supply
 *		 unit will then call resupply, taking more time)
 *
 * May want to put code to resupply with SAMs here, later --ts
 */

void
resupply_all(struct lndstr *lp)
{
    if (!opt_NOFOOD)
	resupply_commod(lp, I_FOOD);
    resupply_commod(lp, I_SHELL);
}

/*
 * If the unit has less than it's minimum level of a
 * certain commodity, fill it, to the best of our abilities.
 */

void
resupply_commod(struct lndstr *lp, i_type type)
{
    int amt;
    struct shpstr ship;

    /* Ok, do we now have enough? */
    amt = get_minimum(lp, type) - lp->lnd_item[type];
    if (amt > 0) {
	lp->lnd_item[type] += supply_commod(lp->lnd_own,
					    lp->lnd_x, lp->lnd_y,
					    type, amt);
	amt = get_minimum(lp, type) - lp->lnd_item[type];
    }
    /* Now, check again to see if we have enough. */
    if (amt > 0) {
	/* Are we on a ship?  if so, try to get it from the ship first. */
	if (lp->lnd_ship >= 0) {
	    getship(lp->lnd_ship, &ship);
	    /* Now, determine how much we can get */
	    if (amt > ship.shp_item[type])
		amt = ship.shp_item[type];
	    /* Now, add and subtract */
	    lp->lnd_item[type] += amt;
	    ship.shp_item[type] -= amt;
	    putship(lp->lnd_ship, &ship);
	}
    }
}

/*
 * Actually get the commod
 */
int
supply_commod(int own, int x, int y, i_type type, int total_wanted)
{
    if (total_wanted <= 0)
	return 0;
    return s_commod(own, x, y, type, total_wanted, !player->simulation);
}

/*
 * Just return the number you COULD get, without doing it
 */
static int
try_supply_commod(int own, int x, int y, i_type type, int total_wanted)
{
    if (total_wanted <= 0)
	return 0;

    return s_commod(own, x, y, type, total_wanted, 0);
}

/* Get supplies of a certain type */
static int
s_commod(int own, int x, int y, i_type type, int total_wanted,
	 int actually_doit)
{
    int wanted = total_wanted;
    int gotten = 0, lookrange;
    struct sctstr sect, dest;
    struct nstr_sect ns;
    struct nstr_item ni;
    struct lchrstr *lcp;
    struct shpstr ship;
    struct lndstr land;
    /* leave at least 1 military in sectors/ships */
    int minimum = (type == I_MILIT ? 1 : 0);
    int can_move;
    double move_cost, weight, mobcost;
    int packing;
    struct dchrstr *dp;
    struct ichrstr *ip;
    char buf[1024];

    /* try to get it from sector we're in */
    getsect(x, y, &dest);
    getsect(x, y, &sect);
    if (sect.sct_own == own) {
	if (sect.sct_item[type] - wanted >= minimum) {
	    sect.sct_item[type] -= wanted;
	    if (actually_doit)
		putsect(&sect);
	    return total_wanted;
	} else if (sect.sct_item[type] - minimum > 0) {
	    gotten += sect.sct_item[type] - minimum;
	    wanted -= sect.sct_item[type] - minimum;
	    sect.sct_item[type] = minimum;
	    if (actually_doit)
		putsect(&sect);
	}
    }
    /* look for a headquarters or warehouse */
    lookrange = tfact(own, 10.0);
    snxtsct_dist(&ns, x, y, lookrange);
    while (nxtsct(&ns, &sect) && wanted) {
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
	if (!BestLandPath(buf, &dest, &sect, &move_cost, MOB_MOVE))
	    continue;
	if (!opt_NOFOOD && type == I_FOOD)
	    minimum = 1 + (int)ceil(food_needed(sect.sct_item,
						etu_per_update));
	if (sect.sct_item[type] <= minimum) {
	    /* Don't bother... */
	    continue;
	}
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
	    n = roundavg(total_wanted * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;

	    if (actually_doit)
		putsect(&sect);

	    return total_wanted;
	} else if (can_move > 0) {
	    int n;
	    gotten += can_move;
	    wanted -= can_move;
	    sect.sct_item[type] -= can_move;

	    /* take off mobility for delivering sect */
	    n = roundavg(can_move * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;

	    if (actually_doit)
		putsect(&sect);
	}
    }

    /* look for an owned ship in a harbor */
    snxtitem_dist(&ni, EF_SHIP, x, y, lookrange);

    while (nxtitem(&ni, &ship) && wanted) {
	if (ship.shp_own != own)
	    continue;

	if (!(mchr[(int)ship.shp_type].m_flags & M_SUPPLY))
	    continue;
	getsect(ship.shp_x, ship.shp_y, &sect);
	if (sect.sct_type != SCT_HARBR)
	    continue;
	if (sect.sct_effic < 2)
	    continue;
	if (!BestLandPath(buf, &dest, &sect, &move_cost, MOB_MOVE))
	    continue;
	if (!opt_NOFOOD && type == I_FOOD)
	    minimum = 1 + (int)ceil(food_needed(ship.shp_item,
						etu_per_update));
	if (ship.shp_item[type] <= minimum) {
	    /* Don't bother... */
	    continue;
	}
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
		putship(ship.shp_uid, &ship);
		putsect(&sect);
	    }
	    return total_wanted;
	} else if (can_move > 0) {
	    int n;
	    gotten += can_move;
	    wanted -= can_move;
	    ship.shp_item[type] -= can_move;

	    n = roundavg(can_move * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= n;

	    if (actually_doit) {
		putship(ship.shp_uid, &ship);
		putsect(&sect);
	    }
	}
    }

    /* look for an owned supply unit */
    snxtitem_dist(&ni, EF_LAND, x, y, lookrange);

    while (nxtitem(&ni, &land) && wanted) {
	int min;

	if (land.lnd_own != own)
	    continue;

	lcp = &lchr[(int)land.lnd_type];
	if (!(lcp->l_flags & L_SUPPLY))
	    continue;

	if (land.lnd_item[type] <= get_minimum(&land, type))
	    continue;

	getsect(land.lnd_x, land.lnd_y, &sect);
	if (!BestLandPath(buf, &dest, &sect, &move_cost, MOB_MOVE))
	    continue;

	if ((land.lnd_ship >= 0) && (sect.sct_type != SCT_HARBR))
	    continue;

	if ((land.lnd_ship >= 0) && (sect.sct_effic < 2))
	    continue;

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

	    land.lnd_item[type] =
		save.lnd_item[type] + s_commod(own, land.lnd_x, land.lnd_y,
					       type, wanted, actually_doit);
	    if (actually_doit)
		putland(land.lnd_uid, &land);
	    else
		putland(save.lnd_uid, &save);
	}

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

	    /* resupply the supply unit */
	    resupply_commod(&land, type);

	    land.lnd_mobil -= roundavg(wanted * weight * move_cost);

	    if (actually_doit)
		putland(land.lnd_uid, &land);
	    return total_wanted;
	} else if (can_move > 0) {
	    gotten += can_move;
	    wanted -= can_move;
	    land.lnd_item[type] -= can_move;

	    land.lnd_mobil -= roundavg(can_move * weight * move_cost);

	    if (actually_doit)
		putland(land.lnd_uid, &land);
	}
    }

    /* We've done the best we could */
    /* return the number gotten */
    return gotten;
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
has_supply(struct lndstr *lp)
{
    int shells_needed, shells, keepshells;
    int food, food_needed, keepfood;

    if (!opt_NOFOOD) {
	food_needed = get_minimum(lp, I_FOOD);
	food = keepfood = lp->lnd_item[I_FOOD];
	if (food < food_needed) {
	    lp->lnd_item[I_FOOD] = 0;
	    putland(lp->lnd_uid, lp);
	    food += try_supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y,
				      I_FOOD, (food_needed - food));
	    lp->lnd_item[I_FOOD] = keepfood;
	    putland(lp->lnd_uid, lp);
	}
	if (food < food_needed)
	    return 0;

    }

    shells_needed = lchr[lp->lnd_type].l_ammo;
    shells = keepshells = lp->lnd_item[I_SHELL];
    if (shells < shells_needed) {
	lp->lnd_item[I_SHELL] = 0;
	putland(lp->lnd_uid, lp);
	shells += try_supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y,
				    I_SHELL, (shells_needed - shells));
	lp->lnd_item[I_SHELL] = keepshells;
	putland(lp->lnd_uid, lp);
    }

    if (shells < shells_needed)
	return 0;

    return 1;
}
