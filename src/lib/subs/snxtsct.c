/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  snxtsct.c: Arrange sector selection using either distance or area
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1989
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "file.h"
#include "com.h"
#include "prototypes.h"
#include "optlist.h"

/*
 * setup the nstr_sect structure for sector selection.
 * can select on either NS_ALL, NS_AREA, or NS_RANGE
 * iterate thru the "condarg" string looking
 * for arguments to compile into the nstr.
 */
int
snxtsct(register struct nstr_sect *np, s_char *str)
{
    register s_char *cp;
    struct range range;
    coord cx, cy;
    int dist;
    s_char buf[1024];
    struct range wr;

    if (str == 0 || *str == 0) {
	if ((str = getstring("(sects)? ", buf)) == 0)
	    return 0;
    }
    wr.lx = -WORLD_X / 2;
    wr.ly = -WORLD_Y / 2;
    wr.hx = WORLD_X / 2;
    wr.hy = WORLD_Y / 2;
    wr.width = wr.height = 0;
    switch (sarg_type(str)) {
    case NS_AREA:
	if (!sarg_area(str, &range))
	    return 0;
	snxtsct_area(np, &range);
	break;
    case NS_DIST:
	if (!sarg_range(str, &cx, &cy, &dist))
	    return 0;
	snxtsct_dist(np, cx, cy, dist);
	break;
    case NS_ALL:
	/* fake "all" by doing a world-sized area query */
	snxtsct_area(np, &wr);
	break;
    default:
	return 0;
    }
    if (player->condarg == 0)
	return 1;
    cp = player->condarg;
    while ((cp = nstr_comp(np->cond, &np->ncond, EF_SECTOR, cp)) && *cp) ;
    if (cp == 0)
	return 0;
    return 1;
}

/*
 * The rest of these (snxtsct_all, snxtsct_area, etc, have been moved
 * into the common lib, since they don't use condargs, and are useful
 * elsewhere (update, chiefly). ---ts
 *
 */
