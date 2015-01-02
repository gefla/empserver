/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  rang.c: Edit range of plane/land unit
 *
 *  Known contributors to this file:
 *     Jeff Bailey
 */

#include <config.h>

#include "commands.h"
#include "plane.h"

int
range(void)
{
    struct nstr_item np;
    struct plnstr plane;
    int i, rmax;
    char *p;
    char buf[1024];

    if (!snxtitem(&np, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&np, &plane)) {
	if (!player->owner || plane.pln_own == 0)
	    continue;
	p = getstarg(player->argp[2], "New range? ", buf);
	if (!p)
	    return RET_FAIL;
	if (!check_plane_ok(&plane))
	    return RET_SYN;
	if ((i = atoi(p)) < 0)
	    continue;
	rmax = pln_range_max(&plane);
	plane.pln_range = MIN(rmax, i);
	pr("Plane %d range changed to %d\n",
	   plane.pln_uid, plane.pln_range);

	putplane(plane.pln_uid, &plane);
    }

    return RET_OK;
}
