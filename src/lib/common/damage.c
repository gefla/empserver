/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  damage.c: Damage stuff.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1997
 */

#include "misc.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "nuke.h"
#include "nsc.h"
#include <fcntl.h>
#include "optlist.h"
#include "damage.h"
#include "common.h"
#include "gen.h"
#include "subs.h"

void
item_damage(int pct, short *item)
{
    int lose;
    i_type i;

    for (i = I_NONE + 1; i <= I_MAX; ++i) {
	if (opt_SUPER_BARS && i == I_BAR)
	    continue;
	lose = roundavg((double)item[i] * pct * 0.01);
	if (i == I_CIVIL || i == I_MILIT || i == I_UW)
	    lose = ldround(people_damage * lose, 1);
	item[i] = item[i] >= lose ? item[i] - lose : 0;
    }
}

void
ship_damage(struct shpstr *sp, int dam)
{
    if (dam <= 0)
	return;
    if (dam > 100)
	dam = 100;

    mpr(sp->shp_own, "\t%s takes %d\n", prship(sp), dam);

    sp->shp_effic = damage((int)sp->shp_effic, dam);
    if (sp->shp_mobil > 0)
	sp->shp_mobil = damage((int)sp->shp_mobil, dam);
    if (opt_FUEL && sp->shp_fuel)
	sp->shp_fuel = damage((int)sp->shp_fuel, dam);
    item_damage(dam, sp->shp_item);
}

void
shipdamage(struct shpstr *sp, int dam)
{
    ship_damage(sp, (int)(dam / (1.0 + sp->shp_armor / 100.0)));
}

void
land_damage(struct lndstr *lp, int dam)
{
    if (dam <= 0)
	return;
    if (dam > 100)
	dam = 100;

    mpr(lp->lnd_own, "\t%s takes %d\n", prland(lp), dam);
    if (lchr[(int)lp->lnd_type].l_flags & L_SPY) {
	/* Spies die! */
	lp->lnd_effic = 0;
    } else {
	lp->lnd_effic = damage((int)lp->lnd_effic, dam);
	if (lp->lnd_mobil > 0)
	    lp->lnd_mobil = damage((int)lp->lnd_mobil, dam);
	if (opt_FUEL && lp->lnd_fuel)
	    lp->lnd_fuel = damage((int)lp->lnd_fuel, dam);
	item_damage(dam, lp->lnd_item);
    }
}

void
landdamage(struct lndstr *lp, int dam)
{
    double damage_factor, m;

    m = (double)land_mob_max;

    /* fortification reduces damage */
    damage_factor = m / (m + ((double)lp->lnd_harden));
    if (damage_factor == 0.0)
	damage_factor = 1.0;

    /* vulnerable units take more damage */
    damage_factor *= lp->lnd_vul / 100.0;

    land_damage(lp, ldround(damage_factor * dam, 1));
}

void
planedamage(struct plnstr *pp, int dam)
{
    if (dam <= 0)
	return;
    if (dam > 100)
	dam = 100;

    mpr(pp->pln_own, "\t%s takes %d\n", prplane(pp), dam);
    pp->pln_effic = damage((int)pp->pln_effic, dam);
    if (pp->pln_mobil > 0)
	pp->pln_mobil = damage((int)pp->pln_mobil, dam);
}

/*
 * nukedamage() actually just calculates damage
 * rather than inflicting it.
 */
int
nukedamage(struct nchrstr *ncp, int range, int airburst)
{
    int dam;
    int rad;

    rad = ncp->n_blast;
    if (airburst)
	rad = (int)(rad * 1.5);
    if (rad < range)
	return 0;
    if (airburst) {
	/* larger area, less center damage */
	dam = (int)((ncp->n_dam * 0.75) - (range * 20));
    } else {
	/* smaller area, more center damage */
	dam = (int)(ncp->n_dam / (range + 1.0));
    }
    if (dam < 5)
	dam = 0;
    return dam;
}

int
damage(int amt, int pct)
{
    int tmp;
    int lost;

    if (amt <= 0)
	return 0;
    tmp = amt * pct;
    lost = tmp / 100;
    if ((random() % 100) < (tmp % 100))
	lost++;
    return amt - lost;
}

/* asymptotic damage to commodities, efficiency, and sectors */
int
effdamage(int amt, int dam)
{
    return damage(amt, PERCENT_DAMAGE(dam));
}

int
commdamage(int amt, int dam, i_type vtype)
{
    int lost;

    if (vtype == I_BAR && opt_SUPER_BARS)
	return amt;

    lost = amt - effdamage(amt, dam);

    if (vtype == I_MILIT || vtype == I_CIVIL || vtype == I_UW)
	lost = ldround(people_damage * lost, 1);
    return amt - lost;
}
