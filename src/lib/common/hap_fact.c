/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2014, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  hap_fact.c: Happiness effect on che
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996
 *     Markus Armbruster, 2006-2013
 */

#include <config.h>

#include "misc.h"
#include "nat.h"
#include "prototypes.h"
#include "xy.h"

double
hap_fact(struct natstr *tnat, struct natstr *vnat)
{
    double hap_fact;

    hap_fact = vnat->nat_level[NAT_HLEV];
    if (hap_fact && tnat->nat_level[NAT_HLEV])
	hap_fact = tnat->nat_level[NAT_HLEV] / hap_fact;
    else if (!hap_fact && !tnat->nat_level[NAT_HLEV])
	hap_fact = 1.0;
    else if (tnat->nat_level[NAT_HLEV])	/* Target has happy, better fighting */
	hap_fact = 2.0;
    else			/* Target has no happy, worse fighting */
	hap_fact = 0.8;
    return LIMIT_TO(hap_fact, 0.8, 2.0);
}

/* Return happiness required to keep NP's citizens happy.  */
double
hap_req(struct natstr *np)
{
    return (np->nat_level[NAT_TLEV] - 40.0) / 40.0
	+ np->nat_level[NAT_ELEV] / 3.0;
}
