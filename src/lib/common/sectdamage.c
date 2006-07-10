/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sectdamage.c: Damage a sector
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 *     Steve McClure, 1996
 */

#include <config.h>

#include "combat.h"
#include "damage.h"
#include "file.h"
#include "land.h"
#include "misc.h"
#include "nat.h"
#include "nsc.h"
#include "optlist.h"
#include "plane.h"
#include "prototypes.h"
#include "sect.h"
#include "ship.h"
#include "xy.h"

int
sect_damage(struct sctstr *sp, int dam, struct emp_qelem *list)
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
    if (opt_EASY_BRIDGES == 0) {
	if (sp->sct_effic < SCT_MINEFF && sp->sct_type == SCT_BHEAD)
	    bridgefall(sp, list);
    } else {
	if (sp->sct_effic < SCT_MINEFF && sp->sct_type == SCT_BSPAN)
	    knockdown(sp, list);
    }
    putsect(sp);
    return eff;
}

int
sectdamage(struct sctstr *sp, int dam, struct emp_qelem *list)
{
    struct nstr_item ni;
    struct lndstr land;
    struct plnstr plane;
    int eff;

    /* Some sectors are harder/easier to kill..   */
    /* Average sector has a dstr of 1, so adjust  */
    /* the damage accordingly. Makes forts a pain */
    dam = ldround(dam / sector_strength(sp), 1);

    eff = sect_damage(sp, PERCENT_DAMAGE(dam), list);

    /* Damage all the land units in the sector */
    /* Units don't take full damage */
    dam = ldround(DPERCENT_DAMAGE(dam * unit_damage), 1);
    if (dam <= 0)
	return eff;

    snxtitem_xy(&ni, EF_LAND, sp->sct_x, sp->sct_y);
    while (nxtitem(&ni, &land)) {
	if (!land.lnd_own)
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
	if (plane.pln_ship >= 0)
	    continue;
	/* Is this plane flying in this list? */
	if (ac_isflying(&plane, list))
	    continue;
	planedamage(&plane, dam);
	putplane(plane.pln_uid, &plane);
    }
    return eff;
}
