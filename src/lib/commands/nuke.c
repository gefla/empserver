/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nuke.c: Display a listing of your nuclear arsenal
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2006-2010
 */

#include <config.h>

#include "commands.h"
#include "nuke.h"
#include "plane.h"

int
nuke(void)
{
    int nnukes, noff;
    struct nstr_item nstr;
    struct nukstr nuk;
    struct plnstr plane;

    if (!snxtitem(&nstr, EF_NUKE, player->argp[1], NULL))
	return RET_SYN;
    nnukes = noff = 0;
    while (nxtitem(&nstr, &nuk)) {
	if (!player->owner || nuk.nuk_own == 0)
	    continue;
	if (nnukes++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("   # nuke type              x,y    s   eff tech carry burst\n");
	}
	if (nuk.nuk_off)
	    noff++;
	if (player->god)
	    pr("%-3d ", nuk.nuk_own);
	pr("%4d %-19.19s ", nstr.cur, nchr[(int)nuk.nuk_type].n_name);
	prxy("%4d,%-4d", nuk.nuk_x, nuk.nuk_y);
	pr(" %1.1s %c%3d%% %4d",
	   &nuk.nuk_stockpile, nuk.nuk_off ? '!' : ' ', nuk.nuk_effic,
	   nuk.nuk_tech);
	if (nuk.nuk_plane >= 0) {
	    getplane(nuk.nuk_plane, &plane);
	    pr("%5dP %s",
	       nuk.nuk_plane,
	       plane.pln_flags & PLN_AIRBURST ? "  air" : "ground");
	}
	pr("\n");
    }

    if (nnukes == 0) {
	if (player->argp[1])
	    pr("%s: No nuke(s)\n", player->argp[1]);
	else
	    pr("%s: No nuke(s)\n", "");
	return RET_FAIL;
    } else {
	pr("%d nuke%s", nnukes, splur(nnukes));
	if (noff)
	    pr(", %d stopped (eff marked with !)", noff);
	pr("\n");
    }

    return RET_OK;
}
