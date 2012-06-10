/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  move.c: Misc. move routines
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include "file.h"
#include "misc.h"
#include "nat.h"
#include "optlist.h"
#include "path.h"
#include "sect.h"
#include "xy.h"

double
sector_mcost(struct sctstr *sp, int mobtype)
{
    double base, cost;

    base = dchr[sp->sct_type].d_mob0;
    if (base < 0)
	return -1.0;

    if (mobtype == MOB_RAIL && opt_RAILWAYS) {
	if (!SCT_HAS_RAIL(sp))
	    return -1;
	mobtype = MOB_MARCH;
    }

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

/* Minimal efficiency for railway and railway extension (opt_RAILWAYS) */
#define SCT_RAIL_EFF 5
#define SCT_RAIL_EXT_EFF 60

/* Is sector SP a railway? */
#define SCT_IS_RAILWAY(sp) \
    (dchr[(sp)->sct_type].d_mob1 == 0 && (sp)->sct_effic >= SCT_RAIL_EFF)
/* May sector SP have a railway extension? */
#define SCT_MAY_HAVE_RAIL_EXT(sp) \
    ((sp)->sct_effic >= SCT_RAIL_EXT_EFF)
/* Does railway sector SP extend railway track into sector TOSP? */
#define SCT_EXTENDS_RAIL(sp, tosp) \
    ((sp)->sct_own == (tosp)->sct_own && SCT_MAY_HAVE_RAIL_EXT(tosp))

int
sct_rail_track(struct sctstr *sp)
{
    int i, res;
    struct sctstr *nsp;

    res = !!SCT_IS_RAILWAY(sp);
    for (i = DIR_FIRST; i <= DIR_LAST; i++) {
	nsp = getsectp(sp->sct_x + diroff[i][0],
		       sp->sct_y + diroff[i][1]);
	if (SCT_IS_RAILWAY(nsp) && SCT_EXTENDS_RAIL(nsp, sp))
	    res++;
    }
    return res;
}
