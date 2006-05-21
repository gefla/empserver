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
sector_mcost(struct sctstr *sp, int do_bonus)
{
    double d;

    if (!(d = dchr[sp->sct_type].d_mcst))
	return -1.0;

/* Note, the best you can get is a 1.0 here. */

    if (do_bonus == MOB_ROAD) {
	d = d / (1.0 + sp->sct_road / 122.0);
    } else if (do_bonus == MOB_RAIL) {
	d = d / (1.0 + sp->sct_rail / 100.0);
    } else {
	if (d < 2.0)
	    d = 2.0;
    }
    if (d < 1.0)
	d = 1.0;
    if (dchr[sp->sct_type].d_mcst < 25)
	d = (d * 100.0 - sp->sct_effic) / 500.0;
/*	d = (200.0 + (d - 3.0) * sp->sct_effic) / 500.0;*/
    else
	d = (d * 10.0 - sp->sct_effic) / 115;

    if (d <= 0.0 || d < MIN_MOBCOST)
	return MIN_MOBCOST;
    return d;
}
