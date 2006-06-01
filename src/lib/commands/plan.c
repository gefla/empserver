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
 *  plan.c: Do a plane report
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 2000
 *     Markus Armbruster, 2006
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "plane.h"
#include "nsc.h"
#include "nuke.h"
#include "file.h"
#include "optlist.h"
#include "commands.h"

int
plan(void)
{
    int nplanes, noff;
    struct nstr_item np;
    struct plnstr plane;

    if (!snxtitem(&np, EF_PLANE, player->argp[1]))
	return RET_SYN;
    nplanes = noff = 0;
    while (nxtitem(&np, &plane)) {
	if (!player->owner || plane.pln_own == 0)
	    continue;
	if (nplanes++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("   #    type                x,y    w   eff  mu def tech ran hard carry special\n");
	}
	if (plane.pln_off)
	    noff++;
	if (player->god)
	    pr("%3d ", plane.pln_own);
	pr("%4d %-19.19s ", np.cur, plchr[(int)plane.pln_type].pl_name);
	prxy("%4d,%-4d", plane.pln_x, plane.pln_y, player->cnum);
	pr(" %1.1s %c%3d%% %3d %3d %4d %3d  %3d",
	   &plane.pln_wing, plane.pln_off ? '!' : ' ', plane.pln_effic,
	   plane.pln_mobil, plane.pln_def, plane.pln_tech,
	   plane.pln_range, plane.pln_harden);
	if (plane.pln_ship >= 0)
	    pr("%5dS", plane.pln_ship);
	else if (plane.pln_land >= 0)
	    pr("%5dL", plane.pln_land);
	else
	    pr("      ");
	if (plane.pln_flags & PLN_SYNCHRONOUS)
	    pr(" geosync");
	else if (plane.pln_flags & PLN_LAUNCHED)
	    pr(" orbit");
	else if (plane.pln_nuketype >= 0)
	    pr(" %-5.5s %c",
	       nchr[(int)plane.pln_nuketype].n_name,
	       plane.pln_flags & PLN_AIRBURST ? 'A' : 'G');
	pr("\n");
    }

    if (nplanes == 0) {
	if (player->argp[1])
	    pr("%s: No plane(s)\n", player->argp[1]);
	else
	    pr("%s: No plane(s)\n", "");
	return RET_FAIL;
    } else {
	pr("%d plane%s", nplanes, splur(nplanes));
	if (noff)
	    pr(", %d stopped (eff marked with !)", noff);
	pr("\n");
    }

    return RET_OK;
}
