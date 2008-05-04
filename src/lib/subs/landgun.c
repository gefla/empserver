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
 *  landgun.c: Fire weapons
 * 
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include "damage.h"
#include "file.h"
#include "land.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"

double
fortgun(int effic, int guns)
{
    double d;
    double g = MIN(guns, 7);

    d = (random() % 30 + 20.0) * (g / 7.0);
    d *= effic / 100.0;
    return d;
}

double
seagun(int effic, int guns)
{
    double d;

    d = 0.0;
    while (guns--)
	d += 10.0 + random() % 6;
    d *= effic * 0.01;
    return d;
}

double
landunitgun(int effic, int guns)
{
    double d;

    d = 0.0;
    while (guns--)
	d += 5.0 + random() % 6;
    d *= effic * 0.01;
    return d;
}

/*
 * Fire from fortress SP.
 * Use ammo, resupply if necessary.
 * Return damage if the fortress fires, else -1.
 */
int
fort_fire(struct sctstr *sp)
{
    int guns = sp->sct_item[I_GUN];
    int shells;

    if (sp->sct_type != SCT_FORTR || sp->sct_effic < FORTEFF)
	return -1;
    if (sp->sct_item[I_MILIT] < 5 || guns == 0)
	return -1;
    shells = sp->sct_item[I_SHELL];
    shells += supply_commod(sp->sct_own, sp->sct_x, sp->sct_y,
			    I_SHELL, 1 - shells);
    if (shells == 0)
	return -1;
    sp->sct_item[I_SHELL] = shells - 1;
    return (int)fortgun(sp->sct_effic, guns);
}

/*
 * Fire from ship SP.
 * Use ammo, resupply if necessary.
 * Return damage if the ship fires, else -1.
 */
int
shp_fire(struct shpstr *sp)
{
    int guns, shells;

    if (sp->shp_effic < 60)
	return -1;
    guns = shp_usable_guns(sp);
    guns = MIN(guns, (sp->shp_item[I_MILIT] + 1) / 2);
    if (guns == 0)
	return -1;
    shells = sp->shp_item[I_SHELL];
    shells += supply_commod(sp->shp_own, sp->shp_x, sp->shp_y,
                           I_SHELL, (guns + 1) / 2 - shells);
    guns = MIN(guns, shells * 2);
    if (guns == 0)
       return -1;
    sp->shp_item[I_SHELL] = shells - (guns + 1) / 2;
    return (int)seagun(sp->shp_effic, guns);
}

/*
 * Drop depth-charges from ship SP.
 * Use ammo, resupply if necessary.
 * Return damage if the ship drops depth-charges, else -1.
 */
int
shp_dchrg(struct shpstr *sp)
{
    int shells, dchrgs;

    if (sp->shp_effic < 60 || (mchr[sp->shp_type].m_flags & M_DCH) == 0)
	return -1;
    if (sp->shp_item[I_MILIT] == 0)
	return -1;
    shells = sp->shp_item[I_SHELL];
    shells += supply_commod(sp->shp_own, sp->shp_x, sp->shp_y,
                           I_SHELL, 2 - shells);
    if (shells == 0)
       return -1;
    dchrgs = MIN(2, shells);
    sp->shp_item[I_SHELL] = shells - dchrgs;
    return (int)seagun(sp->shp_effic, 2 * dchrgs - 1);
}

/*
 * Fire torpedo from ship SP.
 * Use ammo and mobility, resupply if necessary.
 * Return damage if the ship fires, else -1.
 */
int
shp_torp(struct shpstr *sp, int usemob)
{
    int shells;

    if (sp->shp_effic < 60 || (mchr[sp->shp_type].m_flags & M_TORP) == 0)
	return -1;
    if (sp->shp_item[I_MILIT] == 0 || sp->shp_item[I_GUN] == 0)
	return -1;
    if (usemob && sp->shp_mobil <= 0)
	return -1;
    shells = sp->shp_item[I_SHELL];
    shells += supply_commod(sp->shp_own, sp->shp_x, sp->shp_y,
                           I_SHELL, SHP_TORP_SHELLS - shells);
    if (shells < SHP_TORP_SHELLS)
       return -1;
    sp->shp_item[I_SHELL] = shells - SHP_TORP_SHELLS;
    if (usemob)
	sp->shp_mobil -= (int)shp_mobcost(sp) / 2.0;
    return TORP_DAMAGE();
}

/*
 * Fire from land unit LP.
 * Use ammo, resupply if necessary.
 * Return damage if the land unit fires, else -1.
 */
int
lnd_fire(struct lndstr *lp)
{
    int guns, ammo, shells;
    double d;

    if (lp->lnd_effic < LAND_MINFIREEFF)
	return -1;
    if (lp->lnd_ship >= 0 || lp->lnd_land >= 0)
	return -1;
    if (lp->lnd_item[I_MILIT] == 0)
	return -1;
    guns = lnd_dam(lp);
    guns = MIN(guns, lp->lnd_item[I_GUN]);
    if (guns == 0)
	return -1;
    ammo = lchr[lp->lnd_type].l_ammo;
    if (CANT_HAPPEN(ammo == 0))
	ammo = 1;
    shells = lp->lnd_item[I_SHELL];
    shells += supply_commod(lp->lnd_own, lp->lnd_x, lp->lnd_y,
			    I_SHELL, ammo - shells);
    if (shells == 0)
	return -1;
    d = landunitgun(lp->lnd_effic, guns);
    if (shells < ammo) {
	d *= (double)shells / (double)ammo;
	ammo = shells;
    }
    lp->lnd_item[I_SHELL] = shells - ammo;
    return d;
}

/*
 * Return number of guns ship SP can fire.
 */
int
shp_usable_guns(struct shpstr *sp)
{
    return MIN(shp_glim(sp), sp->shp_item[I_GUN]);
}

/*
 * Return effective firing range for range factor RNG at tech TLEV.
 */
static double
effrange(int rng, double tlev)
{
    /* FIXME don't truncate TLEV */
    return techfact((int)tlev, rng / 2.0);
}

/*
 * Return firing range for sector SP.
 */
double
fortrange(struct sctstr *sp)
{
    struct natstr *np = getnatp(sp->sct_own);
    double rng;

    if (sp->sct_type != SCT_FORTR || sp->sct_effic < FORTEFF)
	return -1.0;

    rng = effrange(14.0 * fire_range_factor, np->nat_level[NAT_TLEV]);
    if (sp->sct_effic > 59)
	rng++;
    return rng;
}

/*
 * Return firing range for ship SP.
 */
double
shp_fire_range(struct shpstr *sp)
{
    return effrange(shp_frnge(sp), sp->shp_tech);
}

/*
 * Return torpedo range for ship SP.
 */
double
torprange(struct shpstr *sp)
{
    return effrange(shp_frnge(sp) * 2, sp->shp_tech)
	* sp->shp_effic / 100.0;
}

/*
 * Return hit chance for torpedo from ship SP at range RANGE.
 */
double
shp_torp_hitchance(struct shpstr *sp, int range)
{
    return DTORP_HITCHANCE(range, shp_visib(sp));
}

/*
 * Return firing range for land unit SP.
 */
double
lnd_fire_range(struct lndstr *lp)
{
    return effrange(lnd_frg(lp), lp->lnd_tech);
}

int
roundrange(double r)
{
    return roundavg(r);
}
