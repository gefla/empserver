/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plan.c: Do a plane report
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 2000
 *     Markus Armbruster, 2006-2008
 */

#include <config.h>

#include "commands.h"
#include "nuke.h"
#include "optlist.h"
#include "plane.h"

int
plan(void)
{
    int nplanes, noff;
    struct nstr_item np;
    struct plnstr plane;
    struct nukstr nuke;

    if (!snxtitem(&np, EF_PLANE, player->argp[1], NULL))
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
	prxy("%4d,%-4d", plane.pln_x, plane.pln_y);
	pr(" %1.1s %c%3d%% %3d %3d %4d %3d  %3d",
	   &plane.pln_wing, plane.pln_off ? '!' : ' ', plane.pln_effic,
	   plane.pln_mobil, pln_def(&plane), plane.pln_tech,
	   plane.pln_range, plane.pln_harden);
	if (plane.pln_ship >= 0)
	    pr("%5dS", plane.pln_ship);
	else if (plane.pln_land >= 0)
	    pr("%5dL", plane.pln_land);
	else
	    pr("      ");
	if (pln_is_in_orbit(&plane))
	    pr((plane.pln_flags & PLN_SYNCHRONOUS) ? " geosync" : " orbit");
	else if (getnuke(nuk_on_plane(&plane), &nuke))
	    pr(" %-5.5s %c",
	       nchr[nuke.nuk_type].n_name,
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
