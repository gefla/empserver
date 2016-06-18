/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2016, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  newe.c: Show new sector efficiency (projected)
 *
 *  Known contributors to this file:
 *     Thomas Ruschak, 1993
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "optlist.h"
#include "update.h"

int
newe(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    int nsect;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    player->simulation = 1;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (!sect.sct_off) {
	    natp = getnatp(sect.sct_own);
	    do_feed(&sect, natp, etu_per_update, 1);
	    buildeff(&sect);
	}
	if (nsect++ == 0) {
	    pr("EFFICIENCY SIMULATION\n");
	    pr("   sect  des    projected eff\n");
	}
	prxy("%4d,%-4d", nstr.x, nstr.y);
	pr(" %c", dchr[sect.sct_type].d_mnem);
	pr("    %3d%%\n", sect.sct_effic);
    }
    player->simulation = 0;
    if (nsect == 0) {
	if (player->argp[1])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return RET_OK;
}
