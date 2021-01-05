/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2018, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sectdamage.c: Damage a sector
 *
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2018
 */

#include <config.h>

#include "combat.h"
#include "damage.h"
#include "land.h"
#include "misc.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

int
sect_damage(struct sctstr *sp, int dam)
{
    int eff;

    if (dam <= 0)
	return 0;
    if (dam > 100)
	dam = 100;

    sp->sct_effic = damage(sp->sct_effic, dam);
    sp->sct_avail = damage(sp->sct_avail, dam);
    sp->sct_road = damage(sp->sct_road, dam);
    sp->sct_rail = damage(sp->sct_rail, dam);
    sp->sct_defense = damage(sp->sct_defense, dam);

    eff = dam;

    if (sp->sct_mobil > 0)
	sp->sct_mobil = damage(sp->sct_mobil, dam);
    item_damage(dam, sp->sct_item);
    bridge_damaged(sp);
    putsect(sp);
    return eff;
}

int
sectdamage(struct sctstr *sp, int dam)
{
    struct nstr_item ni;
    struct lndstr land;
    struct plnstr plane;
    int eff;

    /* Some sectors are harder/easier to kill..   */
    /* Average sector has a dstr of 1, so adjust  */
    /* the damage accordingly. Makes forts a pain */
    dam = ldround(dam / sector_strength(sp), 1);

    eff = sect_damage(sp, PERCENT_DAMAGE(dam));

    /* Damage all the land units in the sector */
    /* Units don't take full damage */
    dam = ldround(DPERCENT_DAMAGE(dam * unit_damage), 1);
    if (dam <= 0)
	return eff;

    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &land)) {
	if (!land.lnd_own)
	    continue;
	if (land.lnd_ship >= 0 || land.lnd_land >= 0)
	    continue;
	landdamage(&land, dam);
	putland(land.lnd_uid, &land);
    }

    dam = dam / 7;
    if (dam <= 0)
	return eff;
    snxtitem_xy(&ni, EF_PLANE, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &plane)) {
	if (!plane.pln_own)
	    continue;
	if (plane.pln_flags & PLN_LAUNCHED)
	    continue;
	if (plane.pln_ship >= 0 || plane.pln_land >= 0)
	    continue;
	planedamage(&plane, dam);
	putplane(plane.pln_uid, &plane);
    }
    return eff;
}
