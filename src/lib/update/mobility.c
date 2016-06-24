/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mobility.c: Add mobility to each of the items which accumulate mobility.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998-1999
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "file.h"
#include "game.h"
#include "land.h"
#include "optlist.h"
#include "plane.h"
#include "sect.h"
#include "ship.h"
#include "update.h"

/* Increase mobility of everything for @etus ETUs, update timestamps */
void
mob_inc_all(int etus)
{
    struct sctstr *sectp;
    struct shpstr *sp;
    struct plnstr *pp;
    struct lndstr *lp;
    int i;
    time_t now;

    time(&now);

    for (i = 0; (sectp = getsectid(i)); i++) {
	sectp->sct_timestamp = now;
	if (!opt_MOB_ACCESS)
	    mob_inc_sect(sectp, etus);
    }

    for (i = 0; (sp = getshipp(i)); i++) {
	sp->shp_timestamp = now;
	if (!opt_MOB_ACCESS)
	    mob_inc_ship(sp, etus);
    }

    for (i = 0; (pp = getplanep(i)); i++) {
	pp->pln_timestamp = now;
	if (!opt_MOB_ACCESS)
	    mob_inc_plane(pp, etus);
    }

    for (i = 0; (lp = getlandp(i)); i++) {
	lp->lnd_timestamp = now;
	if (!opt_MOB_ACCESS)
	    mob_inc_land(lp, etus);
    }
}

/* Increase @sp's mobility for @etus ETUs */
void
mob_inc_sect(struct sctstr *sp, int etus)
{
    int value;

    if (CANT_HAPPEN(etus < 0))
	etus = 0;

    if (sp->sct_own == 0)
	return;
    if (sp->sct_type == SCT_WATER || sp->sct_type == SCT_SANCT)
	return;

    value = sp->sct_mobil + ((float)etus * sect_mob_scale);
    if (value > sect_mob_max)
	value = sect_mob_max;
    sp->sct_mobil = value;
}

/* Increase @sp's mobility for @etus ETUs */
void
mob_inc_ship(struct shpstr *sp, int etus)
{
    int value;

    if (CANT_HAPPEN(etus < 0))
	etus = 0;

    if (sp->shp_own == 0)
	return;

    value = sp->shp_mobil + (float)etus * ship_mob_scale;
    if (value > ship_mob_max)
	value = ship_mob_max;
    sp->shp_mobil = (signed char)value;
}

/* Increase @lp's mobility for @etus ETUs */
void
mob_inc_land(struct lndstr *lp, int etus)
{
    int value;

    if (CANT_HAPPEN(etus < 0))
	etus = 0;

    if (lp->lnd_own == 0)
	return;

    value = lp->lnd_mobil + ((float)etus * land_mob_scale);
    if (value > land_mob_max) {
	if (lp->lnd_harden < land_mob_max && !opt_MOB_ACCESS) {
	    /*
	     * Automatic fortification on excess mobility.
	     * Disabled for MOB_ACCESS, because it leads to
	     * excessively deep recursion and thus miserable
	     * performance as the number of land units grows.
	     *
	     * Provide mobility to be used in lnd_fortify()
	     * without overflowing lnd_mobil.
	     */
	    lp->lnd_mobil = land_mob_max;
	    lnd_fortify(lp, value - land_mob_max);
	}
	value = land_mob_max;
    }
    lp->lnd_mobil = value;
}

/* Increase @pp's mobility for @etus ETUs */
void
mob_inc_plane(struct plnstr *pp, int etus)
{
    int value;

    if (CANT_HAPPEN(etus < 0))
	etus = 0;

    if (pp->pln_own == 0)
	return;

    value = pp->pln_mobil + ((float)etus * plane_mob_scale);
    if (value > plane_mob_max)
	value = plane_mob_max;
    pp->pln_mobil = value;
}

/*
 * Credit the turn's remaining MOB_ACCESS mobility.
 * Exactly as if everything was accessed right now.
 */
void
mob_access_all(void)
{
    struct sctstr *sectp;
    struct shpstr *sp;
    struct plnstr *pp;
    struct lndstr *lp;
    int i;

    if (CANT_HAPPEN(!opt_MOB_ACCESS))
	return;

    for (i = 0; (sectp = getsectid(i)); i++)
	mob_inc_sect(sectp, game_reset_tick(&sectp->sct_access));

    for (i = 0; (sp = getshipp(i)); i++)
	mob_inc_ship(sp, game_reset_tick(&sp->shp_access));

    for (i = 0; (pp = getplanep(i)); i++)
	mob_inc_plane(pp, game_reset_tick(&pp->pln_access));

    for (i = 0; (lp = getlandp(i)); i++)
	mob_inc_land(lp, game_reset_tick(&lp->lnd_access));
}
