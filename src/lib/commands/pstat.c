/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  pstat.c: Show the stats of your planes
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"
#include "plane.h"

int
pstat(void)
{
    int nplanes;
    struct nstr_item np;
    struct plnstr plane;

    if (!snxtitem(&np, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    nplanes = 0;
    while (nxtitem(&np, &plane)) {
	if (!player->owner || plane.pln_own == 0)
	    continue;
	if (nplanes++ == 0) {
	    pr("pln# %22.22s    x,y    eff  tech att def acc ran load fuel\n", "plane-type");
	}
	pr("%4d %-22.22s ",
	   plane.pln_uid, plchr[(int)plane.pln_type].pl_name);
	prxy("%4d,%-4d", plane.pln_x, plane.pln_y, player->cnum);
	pr(" %3d%% %4d %3d %3d %3d %3d %4d %4d",
	   plane.pln_effic,
	   plane.pln_tech,
	   pln_att(&plane),
	   pln_def(&plane),
	   pln_acc(&plane),
	   pln_range_max(&plane), pln_load(&plane),
	   plchr[(int)plane.pln_type].pl_fuel);
	pr("\n");
    }

    if (nplanes == 0) {
	if (player->argp[1])
	    pr("%s: No plane(s)\n", player->argp[1]);
	else
	    pr("%s: No plane(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d plane%s\n", nplanes, splur(nplanes));

    return RET_OK;
}
