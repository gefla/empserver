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
 *  lstat.c: Show the stats of your land units
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1996
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "land.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

int
lsta(void)
{
    int nunits;
    struct nstr_item ni;
    struct lndstr land;

    if (!snxtitem(&ni, EF_LAND, player->argp[1]))
	return RET_SYN;

    nunits = 0;
    while (nxtitem(&ni, &land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	count_land_planes(&land);

	if (nunits++ == 0) {
	    pr("     %16.16s                                 s  v  s  r  r  a  f  a  a\n", "");
	    pr("     %16.16s                                 p  i  p  a  n  c  i  m  a\n", "");
	    pr("lnd# %16.16s    x,y    eff tech att def vul  d  s  y  d  g  c  r  m  f\n", "unit-type");
	}
	pr("%4d %-16.16s ", land.lnd_uid, lchr[(int)land.lnd_type].l_name);
	prxy("%4d,%-4d", land.lnd_x, land.lnd_y, player->cnum);
	pr(" %3d%% %3d %1.1f %1.1f %3d ",
	   land.lnd_effic, land.lnd_tech, land.lnd_att, land.lnd_def,
	   land.lnd_vul);
	pr("%2d %2d %2d %2d ",
	   land.lnd_spd, land.lnd_vis, land.lnd_spy, land.lnd_rad);
	pr("%2d %2d %2d %2d %2d ",
	   land.lnd_frg,
	   land.lnd_acc, land.lnd_dam, land.lnd_ammo, land.lnd_aaf);
	pr("\n");
    }
    if (nunits == 0) {
	if (player->argp[1])
	    pr("%s: No unit(s)\n", player->argp[1]);
	else
	    pr("%s: No unit(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d unit%s\n", nunits, splur(nunits));
    return RET_OK;
}
