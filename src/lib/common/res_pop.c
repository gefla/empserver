/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  res_pop.c: Get maximum pop of a sector
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1998
 */

#include "misc.h"
#include "sect.h"
#include "nat.h"
#include "item.h"
#include "xy.h"
#include "optlist.h"
#include "common.h"

int
max_population(float research, int desig, int eff)
{
    int maxpop = dchr[desig].d_maxpop;
    int rmax;

    if (opt_BIG_CITY) {
	/* city efficiency limits maximum population */
	if (dchr[desig].d_pkg == UPKG)
	    maxpop *= 1 + 9.0 * eff / 100;
    }

    if (opt_RES_POP) {
	/* research limits maximum population */
	rmax = maxpop * 0.4
	    + maxpop * 0.6 * (50.0 + 4.0*research) / (200.0 + 3.0*research);
	if (maxpop > rmax)
	    maxpop = rmax;
    }

    if (CANT_HAPPEN(maxpop > ITEM_MAX))
	maxpop = ITEM_MAX;

    return maxpop;
}

int
max_pop(float research, struct sctstr *sp)
{
    return max_population(research, sp->sct_type, sp->sct_effic);
}
