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
 *  supply.c: Supply subroutines
 * 
 *  Known contributors to this file:
 *  
 */

#include "misc.h"
#include "nat.h"
#include "var.h"
#include "retreat.h"
#include "ship.h"
#include "land.h"
#include "sect.h"
#include "news.h"
#include "xy.h"
#include "nsc.h"
#include "path.h"
#include "item.h"
#include "file.h"
#include "options.h"
#include "optlist.h"
#include "player.h"
#include "prototypes.h"

static int get_minimum(struct lndstr *, int);
static int s_commod(int, int, int, int, int, int);

/*
 * We want to get enough guns to be maxed out, enough shells to
 * 	fire once, one update's worth of food, enough fuel for
 *	one update.
 *
 * Firts, try to forage in the sector
 * Second look for a warehouse or headquarters to leech
 * Third, look for a ship we own in a harbor
 * Fourth, look for supplies in a supply unit we own
 * 		(one good reason to do this last is that the supply
 * 		 unit will then call resupply, taking more time)
 *
 * May want to put code to resupply with SAMs here, later --ts
 *
 */

void
resupply_all(struct lndstr *lp)
{
    if (!opt_NOFOOD)
	resupply_commod(lp, I_FOOD);
    resupply_commod(lp, I_SHELL);
    if (opt_FUEL)
	resupply_commod(lp, I_PETROL);
}

/*
 * If the unit has less than it's minimum level of a
 * certain commodity, fill it, to the best of our abilities.
 */

void
resupply_commod(struct lndstr *lp, int type)
{
    int vec[I_MAX + 1];
    int svec[I_MAX + 1];
    int amt;
    struct lchrstr *lcp;
    struct shpstr ship;

    lcp = &lchr[(int)lp->lnd_type];

    getvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
    /* Ok, do we now have enough? */
    if (vec[type] < get_minimum(lp, type)) {
	vec[type] += supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y, type,
				   get_minimum(lp, type) - vec[type]);
	putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
    }
    /* Now, check again to see if we have enough. */
    if (vec[type] < get_minimum(lp, type)) {
	/* Nope.  How much do we need? */
	amt = (get_minimum(lp, type) - vec[type]);
	/* Are we on a ship?  if so, try to get it from the ship first. */
	if (lp->lnd_ship >= 0) {
	    getship(lp->lnd_ship, &ship);
	    getvec(VT_ITEM, svec, (s_char *)&ship, EF_SHIP);
	    /* Now, determine how much we can get */
	    amt = (amt < svec[type]) ? amt : svec[type];
	    /* Now, add and subtract */
	    vec[type] += amt;
	    svec[type] -= amt;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	    putvec(VT_ITEM, svec, (s_char *)&ship, EF_SHIP);
	    putship(lp->lnd_ship, &ship);
	}
    }

    if (opt_FUEL && type == I_PETROL) {
	int fuel_needed = (lp->lnd_fuelu * (((float)etu_per_update
					     * land_mob_scale)) / 10.0);

	while ((lp->lnd_fuel < fuel_needed) && vec[I_PETROL]) {
	    lp->lnd_fuel += 10;
	    if (lp->lnd_fuel > lp->lnd_fuelc)
		lp->lnd_fuel = lp->lnd_fuelc;
	    vec[I_PETROL]--;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	}
    }
}

/*
 * Actually get the commod
 */
int
supply_commod(int own, int x, int y, int type, int total_wanted)
{
    if (total_wanted < 0)
	return 0;
    return s_commod(own, x, y, type, total_wanted, !player->simulation);
}

/*
 * Just return the number you COULD get, without doing it
 */
int
try_supply_commod(int own, int x, int y, int type, int total_wanted)
{
    if (total_wanted < 0)
	return 0;

    return s_commod(own, x, y, type, total_wanted, 0);
}

/* Get supplies of a certain type */
static int
s_commod(int own, int x, int y, int type, int total_wanted,
	 int actually_doit)
{
    int wanted = total_wanted;
    int gotten = 0, lookrange;
    struct sctstr sect, dest;
    int vec[I_MAX + 1];
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
    s_char buf[1024];

    /* try to get it from sector we're in */
    getsect(x, y, &dest);
    getsect(x, y, &sect);
    if (sect.sct_own == own) {
	getvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
	if ((vec[type] - minimum) >= wanted) {
	    vec[type] -= wanted;
	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
		putsect(&sect);
	    }
	    return total_wanted;
	} else if ((vec[type] - minimum) > 0) {
	    gotten += (vec[type] - minimum);
	    wanted -= (vec[type] - minimum);
	    vec[type] = minimum;
	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
		putsect(&sect);
	    }
	}
    }
    /* look for a headquarters or warehouse */
    lookrange = tfact(own, (double)10.0);
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
	if (BestLandPath(buf, &dest, &sect, &move_cost, MOB_ROAD) ==
	    (s_char *)0)
	    continue;
	getvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
	if (!opt_NOFOOD && type == I_FOOD) {
	    minimum = (((double)etu_per_update * eatrate) *
		       (double)(vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW]))
		+ 2;
	}
	if (vec[type] <= minimum) {
	    /* Don't bother... */
	    continue;
	}
	ip = &ichr[type];
	dp = &dchr[sect.sct_type];
	packing = ip->i_pkg[dp->d_pkg];
	if (packing > 1 && sect.sct_effic < 60)
	    packing = 1;
	weight = ((double)ip->i_lbs / (double)packing);
	mobcost = move_cost * weight;
	if (mobcost > 0)
	    can_move = ((double)sect.sct_mobil / mobcost);
	else
	    can_move = vec[type] - minimum;
	if (can_move > (vec[type] - minimum))
	    can_move = (vec[type] - minimum);

	if (can_move >= wanted) {
	    int n;

	    vec[type] -= wanted;

	    /* take off mobility for delivering sect */
	    n = roundavg(total_wanted * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= (u_char)n;

	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
		putsect(&sect);
	    }

	    return total_wanted;
	} else if (can_move > 0) {
	    int n;
	    gotten += can_move;
	    wanted -= can_move;
	    vec[type] -= can_move;

	    /* take off mobility for delivering sect */
	    n = roundavg(can_move * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= (u_char)n;

	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&sect, EF_SECTOR);
		putsect(&sect);
	    }
	}
    }

    /* look for an owned ship in a harbor */
    snxtitem_dist(&ni, EF_SHIP, x, y, lookrange);

    while (nxtitem(&ni, (s_char *)&ship) && wanted) {
	if (ship.shp_own != own)
	    continue;

	if (!(mchr[(int)ship.shp_type].m_flags & M_SUPPLY))
	    continue;
	getsect(ship.shp_x, ship.shp_y, &sect);
	if (sect.sct_type != SCT_HARBR)
	    continue;
	if (sect.sct_effic < 2)
	    continue;
	if (BestLandPath(buf, &dest, &sect, &move_cost, MOB_ROAD) ==
	    (s_char *)0)
	    continue;
	getvec(VT_ITEM, vec, (s_char *)&ship, EF_SHIP);
	if (!opt_NOFOOD && type == I_FOOD)
	    minimum = (((double)etu_per_update * eatrate) *
		       (double)(vec[I_CIVIL] + vec[I_MILIT] + vec[I_UW]))
		+ 2;
	if (vec[type] <= minimum) {
	    /* Don't bother... */
	    continue;
	}
	ip = &ichr[type];
	dp = &dchr[sect.sct_type];
	packing = ip->i_pkg[dp->d_pkg];
	if (packing > 1 && sect.sct_effic < 60)
	    packing = 1;
	weight = ((double)ip->i_lbs / (double)packing);
	mobcost = move_cost * weight;
	if (mobcost > 0)
	    can_move = ((double)sect.sct_mobil / mobcost);
	else
	    can_move = vec[type] - minimum;
	if (can_move > (vec[type] - minimum))
	    can_move = (vec[type] - minimum);
	if (can_move >= wanted) {
	    int n;
	    vec[type] -= wanted;

	    n = roundavg(wanted * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= (u_char)n;
	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&ship, EF_SHIP);
		putship(ship.shp_uid, &ship);
		putsect(&sect);
	    }
	    return total_wanted;
	} else if (can_move > 0) {
	    int n;
	    gotten += can_move;
	    wanted -= can_move;
	    vec[type] -= can_move;

	    n = roundavg(can_move * weight * move_cost);
	    if (n < 0)
		n = 0;
	    if (n > sect.sct_mobil)
		n = sect.sct_mobil;
	    sect.sct_mobil -= (u_char)n;

	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&ship, EF_SHIP);
		putship(ship.shp_uid, &ship);
		putsect(&sect);
	    }
	}
    }

    /* look for an owned supply unit */
    snxtitem_dist(&ni, EF_LAND, x, y, lookrange);

    while (nxtitem(&ni, (s_char *)&land) && wanted) {
	int min;

	if (land.lnd_own != own)
	    continue;

	lcp = &lchr[(int)land.lnd_type];
	if (!(lcp->l_flags & L_SUPPLY))
	    continue;

	getvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
	if (vec[type] <= get_minimum(&land, type))
	    continue;

	getsect(land.lnd_x, land.lnd_y, &sect);
	if (BestLandPath(buf, &dest, &sect, &move_cost, MOB_ROAD) ==
	    (s_char *)0)
	    continue;

	if ((land.lnd_ship >= 0) && (sect.sct_type != SCT_HARBR))
	    continue;

	if ((land.lnd_ship >= 0) && (sect.sct_effic < 2))
	    continue;

	if ((vec[type] - wanted) < get_minimum(&land, type)) {
	    int hold;
	    struct lndstr save;

	    /*
	     * Temporarily zap this unit's store, so the recursion
	     * avoids it.
	     */
	    save = land;
	    hold = vec[type];
	    vec[type] = 0;
	    putvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
	    land.lnd_fuel = 0;
	    putland(land.lnd_uid, &land);

	    hold += s_commod(own, land.lnd_x, land.lnd_y, type, wanted,
			     actually_doit);

	    vec[type] = hold;
	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
		putland(land.lnd_uid, &land);
	    } else
		putland(save.lnd_uid, &save);
	}

	getvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
	min = get_minimum(&land, type);
	ip = &ichr[type];
	weight = ((double)ip->i_lbs);
	mobcost = move_cost * weight;
	if (mobcost > 0)
	    can_move = ((double)land.lnd_mobil / mobcost);
	else
	    can_move = vec[type] - min;
	if (can_move > (vec[type] - min))
	    can_move = (vec[type] - min);

	if (can_move >= wanted) {
	    vec[type] -= wanted;

	    /* resupply the supply unit */
	    resupply_commod(&land, type);

	    land.lnd_mobil -= roundavg(wanted * weight * move_cost);

	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
		putland(land.lnd_uid, &land);
	    }
	    return total_wanted;
	} else if (can_move > 0) {
	    gotten += can_move;
	    wanted -= can_move;
	    vec[type] -= can_move;

	    land.lnd_mobil -= roundavg(can_move * weight * move_cost);

	    if (actually_doit) {
		putvec(VT_ITEM, vec, (s_char *)&land, EF_LAND);
		putland(land.lnd_uid, &land);
	    }
	}
    }

    /* We've done the best we could */
    /* return the number gotten */
    return gotten;
}


/*
 * We want to get enough shells to fire once,
 * one update's worth of food, enough fuel for
 * one update.
 */

static int
get_minimum(struct lndstr *lp, int type)
{
    struct lchrstr *lcp;
    int max, want = 0;

    lcp = &lchr[(int)lp->lnd_type];
    max = vl_find(V_ITEM(type), lcp->l_vtype, lcp->l_vamt, (int)lcp->l_nv);

    switch (type) {
    case I_FOOD:
	if (opt_NOFOOD)
	    return 0;		/* no food reqd, get out */
	want = (((double)etu_per_update * eatrate) *
		(double)total_mil(lp)) + 1;
	break;
    case I_SHELL:
	want = lp->lnd_ammo;
	break;

	/*
	 * return the amount of pet we'd need to get to 
	 * enough fuel for 1 update
	 *
	 */
    case I_PETROL:
	if (opt_FUEL == 0)
	    return 0;
	want = (lp->lnd_fuelu * (((float)etu_per_update *
				  land_mob_scale)) / 10.0);
	want -= lp->lnd_fuel;
	if (want > 0) {
	    double d;
	    d = (double)want / 10.0;
	    want = (int)d;
	    if (want == 0)
		want++;
	}

	max = want;
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
    struct lchrstr *lcp;
    int vec[I_MAX + 1], shells_needed, shells, keepshells;
    int food, food_needed, keepfood;
    int fuel_needed, fuel, petrol_needed, petrol, keeppetrol;

    lcp = &lchr[(int)lp->lnd_type];
    getvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);

    if (!opt_NOFOOD) {
	food_needed = get_minimum(lp, I_FOOD);
	food = keepfood = vec[I_FOOD];
	if (food < food_needed) {
	    vec[I_FOOD] = 0;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	    food += try_supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y,
				      I_FOOD, (food_needed - food));
	    vec[I_FOOD] = keepfood;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	}
	if (food < food_needed)
	    return 0;

    }

    shells_needed = lp->lnd_ammo;
    shells = keepshells = vec[I_SHELL];
    if (shells < shells_needed) {
	vec[I_SHELL] = 0;
	putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	shells += try_supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y,
				    I_SHELL, (shells_needed - shells));
	vec[I_SHELL] = keepshells;
	putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
    }

    if (shells < shells_needed)
	return 0;

    if (opt_FUEL) {
	fuel_needed = lp->lnd_fuelu;

	fuel = lp->lnd_fuel;

	petrol = petrol_needed = 0;

	if (fuel < fuel_needed) {
	    petrol_needed =
		ldround(((double)(fuel_needed - fuel) / 10.0), 1);
	    petrol = keeppetrol = vec[I_PETROL];
	}

	if (petrol < petrol_needed) {
	    vec[I_PETROL] = 0;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	    petrol += try_supply_commod(lp->lnd_own,
					lp->lnd_x, lp->lnd_y,
					I_PETROL,
					(petrol_needed - petrol));
	    vec[I_PETROL] = keeppetrol;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	    fuel += petrol * 10;
	}

	if (fuel < fuel_needed)
	    return 0;
    }
    /* end opt_FUEL */
    return 1;
}

int
use_supply(struct lndstr *lp)
{
    struct lchrstr *lcp;
    int vec[I_MAX + 1], shells_needed, shells, food, food_needed;
    int fuel_needed, fuel, petrol_needed, petrol;

    lcp = &lchr[(int)lp->lnd_type];
    getvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);

    shells_needed = lp->lnd_ammo;
    shells = vec[I_SHELL];
    if (shells < shells_needed) {
	vec[I_SHELL] = 0;
	putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	shells += supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y, I_SHELL,
				(shells_needed - shells));
	vec[I_SHELL] = shells;
    }

    vec[I_SHELL] = max(vec[I_SHELL] - shells_needed, 0);

    if (lp->lnd_frg)		/* artillery */
	goto artillery;

    food_needed = get_minimum(lp, I_FOOD);
    food = vec[I_SHELL];

    if (food < food_needed) {
	vec[I_FOOD] = 0;
	putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	food += supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y, I_FOOD,
			      (food_needed - food));
	vec[I_FOOD] = food;
    }

    vec[I_FOOD] = max(vec[I_FOOD] - food_needed, 0);

    if (opt_FUEL) {
	fuel_needed = lp->lnd_fuelu;
	fuel = lp->lnd_fuel;

	petrol = petrol_needed = 0;

	if (fuel < fuel_needed) {
	    petrol_needed =
		ldround(((double)(fuel_needed - fuel) / 10.0), 1);
	    petrol = vec[I_PETROL];
	}

	if (petrol < petrol_needed) {
	    vec[I_PETROL] = 0;
	    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
	    petrol += supply_commod(lp->lnd_own,
				    lp->lnd_x, lp->lnd_y,
				    I_PETROL, (petrol_needed - petrol));
	    vec[I_PETROL] = petrol;
	}

	if (petrol_needed) {
	    if (petrol >= petrol_needed) {
		vec[I_PETROL] = max(vec[I_PETROL] - petrol_needed, 0);
		lp->lnd_fuel += petrol_needed * 10;
	    } else {
		lp->lnd_fuel += vec[I_PETROL] * 10;
		vec[I_PETROL] = 0;
	    }
	}

	lp->lnd_fuel = max(lp->lnd_fuel - fuel_needed, 0);
    }
    /* end opt_FUEL */
  artillery:
    putvec(VT_ITEM, vec, (s_char *)lp, EF_LAND);
    putland(lp->lnd_uid, lp);
    return 1;
}
