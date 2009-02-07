/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mobility.c: Add mobility to each of the items which accumulate mobility.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998-1999
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include "game.h"
#include "land.h"
#include "plane.h"
#include "server.h"
#include "ship.h"
#include "update.h"

static int do_upd_checking;

static void do_mob_land(struct lndstr *, int);
static void do_mob_plane(struct plnstr *, int);
static void do_mob_sect(struct sctstr *sp, int etus);
static void do_mob_ship(struct shpstr *, int);

void
sct_do_upd_mob(struct sctstr *sp)
{
    int etus;

    if (do_upd_checking || update_running)
	return;
    if (sp->sct_own == 0)
	return;
    if (sp->sct_type == SCT_SANCT)
	return;
    etus = game_tick_to_now(&sp->sct_access);
    if (etus == 0)
	return;

    do_upd_checking = 1;	/* avoid recursion */
    do_mob_sect(sp, etus);
    do_upd_checking = 0;
}

void
shp_do_upd_mob(struct shpstr *sp)
{
    int etus;

    if (do_upd_checking || update_running)
	return;
    if (sp->shp_own == 0)
	return;
    etus = game_tick_to_now(&sp->shp_access);
    if (etus == 0)
	return;

    do_upd_checking = 1;	/* avoid recursion */
    do_mob_ship(sp, etus);
    do_upd_checking = 0;
}

void
lnd_do_upd_mob(struct lndstr *lp)
{
    int etus;

    if (do_upd_checking || update_running)
	return;
    if (lp->lnd_own == 0)
	return;
    etus = game_tick_to_now(&lp->lnd_access);
    if (etus == 0)
	return;

    do_upd_checking = 1;	/* avoid recursion */
    do_mob_land(lp, etus);
    do_upd_checking = 0;
}

void
pln_do_upd_mob(struct plnstr *pp)
{
    int etus;

    if (do_upd_checking || update_running)
	return;
    if (pp->pln_own == 0)
	return;
    etus = game_tick_to_now(&pp->pln_access);
    if (etus == 0)
	return;

    do_upd_checking = 1;	/* avoid recursion */
    do_mob_plane(pp, etus);
    do_upd_checking = 0;
}

void
mob_sect(void)
{
    struct sctstr *sp;
    int n, etus;
    time_t now;

    time(&now);
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	sp->sct_timestamp = now;
	if (opt_MOB_ACCESS)
	    etus = game_reset_tick(&sp->sct_access);
	else
	    etus = etu_per_update;
	do_mob_sect(sp, etus);
    }
}

static void
do_mob_sect(struct sctstr *sp, int etus)
{
    int value;

    if (CANT_HAPPEN(etus < 0))
	etus = 0;

    if (sp->sct_own == 0)
	return;
    if (sp->sct_type == SCT_SANCT)
	return;

    value = sp->sct_mobil + ((float)etus * sect_mob_scale);
    if (value > sect_mob_max)
	value = sect_mob_max;
    sp->sct_mobil = value;
}

void
mob_ship(void)
{
    struct shpstr *sp;
    int n, etus;
    time_t now;

    time(&now);
    for (n = 0; NULL != (sp = getshipp(n)); n++) {
	sp->shp_timestamp = now;
	if (opt_MOB_ACCESS)
	    etus = game_reset_tick(&sp->shp_access);
	else
	    etus = etu_per_update;
	do_mob_ship(sp, etus);
    }
}

static void
do_mob_ship(struct shpstr *sp, int etus)
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

void
mob_land(void)
{
    struct lndstr *lp;
    int n, etus;
    time_t now;

    time(&now);
    for (n = 0; NULL != (lp = getlandp(n)); n++) {
	lp->lnd_timestamp = now;
	if (opt_MOB_ACCESS)
	    etus = game_reset_tick(&lp->lnd_access);
	else
	    etus = etu_per_update;
	do_mob_land(lp, etus);
    }
}

static void
do_mob_land(struct lndstr *lp, int etus)
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

void
mob_plane(void)
{
    struct plnstr *pp;
    int n, etus;
    time_t now;

    time(&now);
    for (n = 0; NULL != (pp = getplanep(n)); n++) {
	pp->pln_timestamp = now;
	if (opt_MOB_ACCESS)
	    etus = game_reset_tick(&pp->pln_access);
	else
	    etus = etu_per_update;
	do_mob_plane(pp, etus);
    }
}

static void
do_mob_plane(struct plnstr *pp, int etus)
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
