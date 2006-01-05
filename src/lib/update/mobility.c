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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  mobility.c: Add mobility to each of the items which accumulate mobility.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 1998-1999
 */

#include <config.h>

#include "misc.h"
#include "sect.h"
#include "ship.h"
#include "land.h"
#include "plane.h"
#include "nat.h"
#include "file.h"
#include "optlist.h"
#include "update.h"
#include "gen.h"
#include "subs.h"
#include "optlist.h"
#include "server.h"


int updating_mob = 1;

static int timestamp_fixing;
static int do_upd_checking = 0;

static void do_mob_land(struct lndstr *, int);
static void do_mob_plane(struct plnstr *, int);
static void do_mob_sect(struct sctstr *sp, int etus);
static void do_mob_ship(struct shpstr *, int);

static int
increase_mob(time_t *counter, float mult)
{
    time_t secs;
    time_t now;
    time_t left;
    int newetus;
    float newmob;
    int inewmob;

    time(&now);

    secs = now - *counter;
    if (secs < 1 || secs < s_p_etu)
	return 0;
    newetus = (int)(secs / s_p_etu);
    if (newetus < 1)
	return 0;
    left = (secs % s_p_etu);
    do {
	newmob = (float)(newetus * mult);
	inewmob = (int)(newetus * mult);
	if (newmob == inewmob || newetus > 7)
	    break;
	newetus--;
	left += s_p_etu;
    } while (newetus > 0);
    if (newetus <= 0)
	return 0;

    time(counter);
    *counter = *counter - left;

    if (updating_mob)
	return newetus;
    return 0;
}

void
update_timestamps(time_t lastsavedtime)
{
    struct shpstr *shipp;
    struct sctstr *sectp;
    struct lndstr *landp;
    struct plnstr *planep;
    int n;
    time_t now;
    time_t delta;

    timestamp_fixing = 1;
    time(&now);
    delta = now - lastsavedtime;
    for (n = 0; (shipp = getshipp(n)); n++)
	shipp->shp_access += delta;
    for (n = 0; (sectp = getsectid(n)); n++)
	sectp->sct_access += delta;
    for (n = 0; (landp = getlandp(n)); n++)
	landp->lnd_access += delta;
    for (n = 0; (planep = getplanep(n)); n++)
	planep->pln_access += delta;
    timestamp_fixing = 0;
}

void
update_all_mob(void)
{
    struct shpstr *shipp;
    struct sctstr *sectp;
    struct lndstr *landp;
    struct plnstr *planep;
    int n;

    n = 0;
    while (1) {
	do_upd_checking = 1;
	shipp = getshipp(n);
	sectp = getsectid(n);
	landp = getlandp(n);
	planep = getplanep(n);
	do_upd_checking = 0;
	if (shipp)
	    shp_do_upd_mob(shipp);
	if (sectp)
	    sct_do_upd_mob(sectp);
	if (landp)
	    lnd_do_upd_mob(landp);
	if (planep)
	    pln_do_upd_mob(planep);
	if (!shipp && !sectp && !landp && !planep)
	    break;
	n++;
    }
    do_upd_checking = 0;
}

void
sct_do_upd_mob(struct sctstr *sp)
{
    int etus;

    if (do_upd_checking || timestamp_fixing || update_pending)
	return;
    if (sp->sct_own == 0)
	return;
    if (sp->sct_type == SCT_SANCT)
	return;
    if ((etus = increase_mob(&sp->sct_access, sect_mob_scale)) == 0)
	return;
    do_upd_checking = 1;
    do_mob_sect(sp, etus);
/*    putsect(sp);*/
    do_upd_checking = 0;
}

void
shp_do_upd_mob(struct shpstr *sp)
{
    int etus;

    if (do_upd_checking || timestamp_fixing || update_pending)
	return;
    if (sp->shp_own == 0)
	return;
    if ((etus = increase_mob(&sp->shp_access, ship_mob_scale)) == 0)
	return;
    do_upd_checking = 1;
    do_mob_ship(sp, etus);
    do_upd_checking = 0;
}

void
lnd_do_upd_mob(struct lndstr *lp)
{
    int etus;

    if (do_upd_checking || timestamp_fixing || update_pending)
	return;
    if (lp->lnd_own == 0)
	return;
    if ((etus = increase_mob(&lp->lnd_access, land_mob_scale)) == 0)
	return;

    do_upd_checking = 1;
    do_mob_land(lp, etus);
    do_upd_checking = 0;
}

void
pln_do_upd_mob(struct plnstr *pp)
{
    int etus;

    if (do_upd_checking || timestamp_fixing || update_pending)
	return;
    if (pp->pln_own == 0)
	return;
    if ((etus = increase_mob(&pp->pln_access, plane_mob_scale)) == 0)
	return;

    do_upd_checking = 1;
    do_mob_plane(pp, etus);
    do_upd_checking = 0;
}

void
mob_sect(int etus)
{
    struct sctstr *sp;
    int n;
    time_t now;

    time(&now);
    for (n = 0; NULL != (sp = getsectid(n)); n++) {
	sp->sct_timestamp = now;
	if (opt_MOB_ACCESS)
	    sct_do_upd_mob(sp);
	else
	    do_mob_sect(sp, etus);
    }
}

static void
do_mob_sect(struct sctstr *sp, int etus)
{
    int value;

    if (sp->sct_own == 0)
	return;
    if (sp->sct_type == SCT_SANCT)
	return;
    /* Do we have to even bother? */
    if (sp->sct_mobil >= sect_mob_max) {
	/* No, so set just in case and then return */
	sp->sct_mobil = sect_mob_max;
	return;
    }
    value = sp->sct_mobil + ((float)etus * sect_mob_scale);
    if (value > sect_mob_max)
	value = sect_mob_max;
    sp->sct_mobil = value;
}

void
mob_ship(int etus)
{
    struct shpstr *sp;
    int n;
    time_t now;

    time(&now);
    for (n = 0; NULL != (sp = getshipp(n)); n++) {
	sp->shp_timestamp = now;
	if (opt_MOB_ACCESS)
	    shp_do_upd_mob(sp);
	else
	    do_mob_ship(sp, etus);
    }
}

static void
do_mob_ship(struct shpstr *sp, int etus)
{
    int newfuel = 0;
    int value;
    int can_add, have_fuel_for, total_add;
    double d;

    if (sp->shp_own == 0)
	return;

    /* Do we even have to bother updating this mobility? */
    if (sp->shp_mobil >= ship_mob_max) {
	/* No, so don't.  Just set it to max (just in case) and
	   return. */
	sp->shp_mobil = ship_mob_max;
	return;
    }

    if (opt_FUEL == 0) {	/* only a bit to do ... */
	value = sp->shp_mobil + ((float)etus * ship_mob_scale);
	if (value > ship_mob_max)
	    value = ship_mob_max;
	sp->shp_mobil = value;
	return;			/* so we ship the FUEL stuff */
    }

    /* opt_FUEL in force */
    if (mchr[(int)sp->shp_type].m_fuelu == 0) {
	value = sp->shp_mobil + ((float)etus * ship_mob_scale);
	if (value > ship_mob_max)
	    value = ship_mob_max;
	sp->shp_mobil = (s_char)value;
    } else {
	can_add = ship_mob_max - sp->shp_mobil;
	if (can_add > ((float)etus * ship_mob_scale))
	    can_add = ((float)etus * ship_mob_scale);
	have_fuel_for = ldround((((double)sp->shp_fuel /
				  (double)mchr[(int)sp->shp_type].
				  m_fuelu) * (double)fuel_mult), 1);

	if (can_add > have_fuel_for) {
	    int need;
	    need = can_add - have_fuel_for;
	    d = (double)need;
	    d *= (double)mchr[(int)sp->shp_type].m_fuelu;
	    d /= (double)fuel_mult;
	    d /= 5.0;
	    if ((d - (int)d) > 0.0)
		d++;
	    need = (int)d;
	    newfuel = supply_commod(sp->shp_own, sp->shp_x,
				    sp->shp_y, I_PETROL, need);
	    sp->shp_fuel += (u_char)(newfuel * 5);
	}

	have_fuel_for = ldround((((double)sp->shp_fuel /
				  (double)mchr[(int)sp->shp_type].
				  m_fuelu) * (double)fuel_mult), 1);

	if (can_add > have_fuel_for) {
	    int need;
	    need = can_add - have_fuel_for;
	    d = (double)need;
	    d *= (double)mchr[(int)sp->shp_type].m_fuelu;
	    d /= (double)fuel_mult;
	    d /= 50.0;
	    if ((d - (int)d) > 0.0)
		d++;
	    need = (int)d;
	    newfuel = supply_commod(sp->shp_own, sp->shp_x,
				    sp->shp_y, I_OIL, need);
	    sp->shp_fuel += (u_char)(newfuel * 50);
	}

	have_fuel_for = ldround((((double)sp->shp_fuel /
				  (double)mchr[(int)sp->shp_type].
				  m_fuelu) * (double)fuel_mult), 1);

	if (can_add > have_fuel_for)
	    total_add = have_fuel_for;
	else
	    total_add = can_add;
	d = (double)total_add;
	d *= (double)mchr[(int)sp->shp_type].m_fuelu;
	d /= (double)fuel_mult;
	sp->shp_fuel -= (u_char)ldround(d, 1);
	sp->shp_fuel = (u_char)min(sp->shp_fuel,
				   mchr[(int)sp->shp_type].m_fuelc);
	sp->shp_mobil += (s_char)total_add;
    }
}

void
mob_land(int etus)
{
    struct lndstr *lp;
    int n;
    time_t now;

    time(&now);
    for (n = 0; NULL != (lp = getlandp(n)); n++) {
	lp->lnd_timestamp = now;
	if (opt_MOB_ACCESS)
	    lnd_do_upd_mob(lp);
	else
	    do_mob_land(lp, etus);
    }
}

static void
do_mob_land(struct lndstr *lp, int etus)
{
    int newfuel = 0;
    int value;
    int can_add, have_fuel_for, total_add;
    double d;

    if (lp->lnd_own == 0)
	return;

    if (lp->lnd_mobil >= land_mob_max) {
	lp->lnd_mobil = land_mob_max;
	if (lp->lnd_harden >= land_mob_max) {
	    lp->lnd_harden = land_mob_max;
	    return;
	}
    }

    if (opt_FUEL == 0) {	/* just some bits and pieces */
	value = lp->lnd_mobil + ((float)etus * land_mob_scale);
	if (value > land_mob_max) {
	    lnd_fortify(lp, value - land_mob_max);
	    value = land_mob_max;
	}
	lp->lnd_mobil = value;

	return;			/* Done! */
    }

    /* opt_FUEL in force ... */
    if (lp->lnd_fuelu == 0) {
	value = lp->lnd_mobil + ((float)etus * land_mob_scale);
	if (value > land_mob_max) {
	    lnd_fortify(lp, value - land_mob_max);
	    value = land_mob_max;
	}
	lp->lnd_mobil = value;

    } else {

	can_add = land_mob_max - lp->lnd_mobil;

	if (can_add > ((float)etus * land_mob_scale))
	    can_add = ((float)etus * land_mob_scale);

	have_fuel_for = (lp->lnd_fuel / lp->lnd_fuelu) * fuel_mult;

	if (can_add > have_fuel_for) {
	    int need;
	    need = can_add - have_fuel_for;
	    d = (double)need;
	    d *= (double)lp->lnd_fuelu;
	    d /= (double)fuel_mult;
	    d /= 5.0;
	    if ((d - (int)d) > 0.0)
		d++;
	    need = (int)d;
	    newfuel = supply_commod(lp->lnd_own, lp->lnd_x,
				    lp->lnd_y, I_PETROL, need);
	    lp->lnd_fuel += (u_char)(newfuel * 5);
	}

	have_fuel_for = (lp->lnd_fuel / lp->lnd_fuelu) * fuel_mult;

	if (can_add > have_fuel_for) {
	    int need;
	    need = can_add - have_fuel_for;
	    d = (double)need;
	    d *= (double)lp->lnd_fuelu;
	    d /= (double)fuel_mult;
	    d /= 50.0;
	    if ((d - (int)d) > 0.0)
		d++;
	    need = (int)d;
	    newfuel = supply_commod(lp->lnd_own, lp->lnd_x,
				    lp->lnd_y, I_OIL, need);
	    lp->lnd_fuel += (u_char)(newfuel * 50);
	}

	have_fuel_for = (lp->lnd_fuel / lp->lnd_fuelu) * fuel_mult;

	if (can_add > have_fuel_for) {
	    total_add = have_fuel_for;
	} else
	    total_add = can_add;
	d = (double)total_add;
	d *= (double)lp->lnd_fuelu;
	d /= (double)fuel_mult;
	lp->lnd_fuel -= (u_char)ldround(d, 1);
	lp->lnd_fuel = (u_char)min(lp->lnd_fuel, lp->lnd_fuelc);
	if (total_add + lp->lnd_mobil > land_mob_max) {
	    total_add = land_mob_max - lp->lnd_mobil;
	}
	/* no automatic fortification here, as it would cost fuel */

	lp->lnd_mobil += (s_char)total_add;
    }
}

void
mob_plane(int etus)
{
    struct plnstr *pp;
    int n;
    time_t now;

    time(&now);
    for (n = 0; NULL != (pp = getplanep(n)); n++) {
	pp->pln_timestamp = now;
	if (opt_MOB_ACCESS)
	    pln_do_upd_mob(pp);
	else
	    do_mob_plane(pp, etus);
    }
}

static void
do_mob_plane(struct plnstr *pp, int etus)
{
    int value;

    if (pp->pln_own == 0)
	return;
    if (pp->pln_mobil >= plane_mob_max) {
	pp->pln_mobil = plane_mob_max;
	return;
    }

    value = pp->pln_mobil + ((float)etus * plane_mob_scale);
    if (value > plane_mob_max)
	value = plane_mob_max;
    pp->pln_mobil = value;
}
