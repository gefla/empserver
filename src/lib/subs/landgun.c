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
 *  landgun.c: Return values for land and ship gun firing damages
 * 
 *  Known contributors to this file:
 *  
 */

#include <config.h>

#include "misc.h"
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "prototypes.h"

double
landgun(int effic, int guns)
{
    double d;
    double g = (double)MIN(guns, 7);

    d = ((double)(random() % 30) + 20.0) * ((double)g / 7.0);
    d *= (double)effic;
    d /= 100.0;
    return d;
}

double
seagun(int effic, int guns)
{
    double d;

    d = 0.0;
    while (guns--)
	d += 10.0 + (double)(random() % 6);
    d *= (double)effic * 0.01;
    return d;
}

double
landunitgun(int effic, int shots, int guns, int ammo, int shells)
{
    double d = 0.0;

    shots = MIN(shots, guns);
    while (shots-- > 0)
	d += 5.0 + (double)(random() % 6);
    d *= (double)effic * 0.01;
    if (shells < ammo && ammo != 0)
	d *= (double)shells / (double)ammo;
    return d;
}

/*
 * Return effective firing range for range factor RNG at tech TLEV.
 */
double
effrange(int rng, double tlev)
{
    /* FIXME don't truncate TLEV */
    return techfact((int)tlev, rng / 2.0);
}

/*
 * Return torpedo range for ship SP.
 */
double
torprange(struct shpstr *sp)
{
    return effrange(sp->shp_frnge * 2, sp->shp_tech)
	* sp->shp_effic / 100.0;
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

int
roundrange(double r)
{
    double f;

    f = r - (int)r;
    if (chance(f))
	return (int)r + 1;
    else
	return (int)r;
}
