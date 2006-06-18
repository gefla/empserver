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
 *  move.c: Misc. move routines
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "misc.h"
#include "xy.h"
#include "sect.h"
#include "path.h"
#include "nat.h"
#include "common.h"

double
sector_mcost(struct sctstr *sp, int mobtype)
{
    double base, cost;

    base = dchr[sp->sct_type].d_mob0;
    if (base < 0)
	return -1.0;

    /* linear function in eff, d_mob0 at 0%, d_mob1 at 100% */
    base += (dchr[sp->sct_type].d_mob1 - base) * sp->sct_effic / 100;
    if (CANT_HAPPEN(base < 0))
	base = 0;

    if (mobtype == MOB_MOVE || mobtype == MOB_MARCH) {
	/* linear function in road, base at 0%, base/10 at 100% */
	cost = base;
	if (intrchr[INT_ROAD].in_enable)
	    cost -= base * 0.009 * sp->sct_road;
    } else if (mobtype == MOB_RAIL) {
	if (!intrchr[INT_RAIL].in_enable || sp->sct_rail <= 0)
	    return -1.0;
	/* linear function in rail, base at 0%, base/100 at 100% */
	cost = base - base * 0.0099 * sp->sct_rail;
    } else {
	CANT_REACH();
	cost = base;
    }
    if (CANT_HAPPEN(cost < 0))
	cost = 0;

    if (mobtype == MOB_MOVE)
	return MAX(cost, 0.001);
    if (sp->sct_own != sp->sct_oldown && sp->sct_mobil <= 0)
	/* slow down land units in newly taken sectors */
	return cost + 0.2;
    return MAX(cost, 0.02);
}

double
speed_factor(double effspd, int tech)
{
    return 480.0 / (effspd + techfact(tech, effspd));
}
