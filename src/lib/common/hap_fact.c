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
 *  hap_fact.c: Happiness effect on che
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <stdio.h>
#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "xy.h"
#include "nat.h"
#include "nsc.h"
#include "common.h"

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
    if (hap_fact > 2.0)
	hap_fact = 2.0;
    if (hap_fact < 0.8)
	hap_fact = 0.8;
    return hap_fact;
}
