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
 *  leve.c: Do a distribution threshold level report
 * 
 *  Known contributors to this file:
 *     David Muir Sharnoff, 1987
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "path.h"
#include "commands.h"

int
leve(void)
{
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;
    int dist[I_MAX + 1];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (nsect++ == 0) {
	    pr("DISTRIBUTION LEVELS\n");
	    if (player->god)
		pr("own");
	    pr("  sect      dst    civ mil  uw food  sh gun pet iron dust bar oil lcm hcm rad\n");
	}
	if (player->god)
	    pr("%2d ", sect.sct_own);
	getvec(VT_DIST, dist, (s_char *)&sect, EF_SECTOR);
	prxy("%3d,%-3d", nstr.x, nstr.y, player->cnum);
	pr(" %c ", dchr[sect.sct_type].d_mnem);
	prxy("%3d,%-3d", sect.sct_dist_x, sect.sct_dist_y, player->cnum);
	pr(" %4d", dist[I_CIVIL]);
	pr("%4d", dist[I_MILIT]);
	pr("%4d", dist[I_UW]);
	pr("%5d", dist[I_FOOD]);
	pr("%4d", dist[I_SHELL]);
	pr("%4d", dist[I_GUN]);
	pr("%4d", dist[I_PETROL]);
	pr("%5d", dist[I_IRON]);
	pr("%5d", dist[I_DUST]);
	pr("%4d", dist[I_BAR]);
	pr("%4d", dist[I_OIL]);
	pr("%4d", dist[I_LCM]);
	pr("%4d", dist[I_HCM]);
	pr("%4d", dist[I_RAD]);
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
