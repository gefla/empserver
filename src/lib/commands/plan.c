/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  plan.c: Do a plane report
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Steve McClure, 2000
 */

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "plane.h"
#include "nsc.h"
#include "nuke.h"
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "commands.h"

int
plan(void)
{
    int nplanes;
    struct nstr_item np;
    struct plnstr plane;

    if (!snxtitem(&np, EF_PLANE, player->argp[1]))
	return RET_SYN;
    nplanes = 0;
    while (nxtitem(&np, (s_char *)&plane)) {
	if (!player->owner || plane.pln_own == 0)
	    continue;
	if (nplanes++ == 0) {
	    if (player->god)
		pr("own ");
	    pr("   #    type                x,y    w  eff  mu def tech ran hard   s/l L");
	    if (opt_ORBIT)
		pr("S");
	    pr("B nuke\n");
	}
	if (player->god)
	    pr("%3d ", plane.pln_own);
	pr("%4d %-19.19s ", np.cur, plchr[(int)plane.pln_type].pl_name);
	prxy("%4d,%-4d", plane.pln_x, plane.pln_y, player->cnum);
	pr(" %c %3d%% %3d %3d %4d %3d  %3d",
	   plane.pln_wing, plane.pln_effic,
	   plane.pln_mobil,
	   plane.pln_def, plane.pln_tech,
	   plane.pln_range, plane.pln_harden);
	if (plane.pln_ship >= 0)
	    pr("%5dS", plane.pln_ship);
	else if (plane.pln_land >= 0)
	    pr("%5dL", plane.pln_land);
	else
	    pr("      ");
	if ((plchr[(int)plane.pln_type].pl_flags & (P_O | P_M)) == P_O) {
	    pr(" %c", (plane.pln_flags & PLN_LAUNCHED) ? 'Y' : 'N');
	    pr("%c",
	       opt_ORBIT ? (plane.
			    pln_flags & PLN_SYNCHRONOUS) ? 'Y' : 'N' :
	       ' ');
	} else
	    pr("   ");
	if (plane.pln_nuketype != -1)
	    pr("%c %-5.5s",
	       plane.pln_flags & PLN_AIRBURST ? 'A' : 'G',
	       nchr[(int)plane.pln_nuketype].n_name);
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
