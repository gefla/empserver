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
 *  peek.c: Show hidden values report (deity)
 *
 *  Known contributors to this file:
 *     Jeff Wallace, 1989
 *     Pat Loney, 1998
 *     Steve McClure, 1998
 */

#include <config.h>

#include "commands.h"

int
peek(void)
{
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0) {
	    pr("    HIDDEN VALUES           che-------  plague----\n");
	    pr("own   sect        eff loyal cnum value  stage time mines\n");
	}
	pr("%3d ", sect.sct_own);
	prxy("%4d,%-4d", nstr.x, nstr.y);
	pr(" %c  %3d%%   %3d  %3d  %3d    %3d   %3d  %3d",
	   dchr[sect.sct_type].d_mnem, sect.sct_effic,
	   sect.sct_loyal, sect.sct_che_target, sect.sct_che,
	   sect.sct_pstage, sect.sct_ptime, sect.sct_mines);
	pr("\n");
    }
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}
