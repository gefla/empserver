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
 *  desi.c: Redesignate sectors
 * 
 *  Known contributors to this file:
 *     Steve McClure, 1998-2000
 *     Markus Armbruster, 2004-2008
 */

#include <config.h>

#include "commands.h"
#include "map.h"
#include "optlist.h"
#include "path.h"

int
desi(void)
{
    int n;
    char *p;
    int breaksanct;
    int cap_x;
    int cap_y;
    int des;
    struct nstr_sect nstr;
    struct sctstr sect;
    struct natstr *natp;
    char prompt[128];
    char buf[1024];
    int changed = 0;

    breaksanct = 0;
    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    natp = getnatp(player->cnum);
    cap_x = natp->nat_xcap;
    cap_y = natp->nat_ycap;
    while (!player->aborted && nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (!player->god && dchr[sect.sct_type].d_cost < 0)
	    continue;
	sprintf(prompt, "%s %d%% %s  desig? ",
		xyas(sect.sct_x, sect.sct_y, player->cnum),
		sect.sct_effic, dchr[sect.sct_type].d_name);
	if ((p = getstarg(player->argp[2], prompt, buf)) == 0)
	    continue;

	if (!check_sect_ok(&sect))
	    continue;

	des = sct_typematch(p);
	if (des < 0 || (((des == SCT_BSPAN) || (des == SCT_BTOWER)) &&
			!player->god)) {
	    pr("No such designation\n"
	       "See \"info Sector-types\" for possible designations\n");
	    return RET_FAIL;
	}
	if (!player->god) {
	    if (des == SCT_WASTE) {
		pr("Only a nuclear device (or %s) can make a %s!\n",
		   cname(0), dchr[des].d_name);
		continue;
	    }
	    if (dchr[des].d_cost < 0) {
		pr("Only %s can make a %s!\n", cname(0), dchr[des].d_name);
		continue;
	    }
	}
	if (sect.sct_type == des && sect.sct_newtype == des)
	    continue;
	if (sect.sct_type == SCT_SANCT)
	    breaksanct++;
	if (des == SCT_HARBR && !sect.sct_coastal) {
	    pr("%s does not border on water.\n",
	       xyas(nstr.x, nstr.y, player->cnum));
	    if (player->god)
		pr("But if it's what you want ...\n");
	    else
		continue;
	}
	if (sect.sct_type == SCT_SANCT && !player->god)
	    continue;
	n = sect.sct_type;
	if ((sect.sct_newtype != des) && (sect.sct_type != des)
	    && dchr[des].d_cost > 0) {
	    if (natp->nat_money < player->dolcost + dchr[des].d_cost) {
		pr("You can't afford a %s!\n", dchr[des].d_name);
		break;
	    }
	    player->dolcost += dchr[des].d_cost;
	}
	if (sect.sct_type != des && (sect.sct_effic < 5 || player->god)) {
	    if (player->god)
		set_coastal(&sect, sect.sct_type, des);
	    sect.sct_type = des;
	    sect.sct_effic = 0;
	    changed += map_set(player->cnum, sect.sct_x, sect.sct_y,
			       dchr[des].d_mnem, 0);
	}
	sect.sct_newtype = des;
	putsect(&sect);
	if (sect.sct_x == cap_x && sect.sct_y == cap_y
	    && des != SCT_CAPIT && des != SCT_SANCT && des != SCT_MOUNT)
	    pr("You have redesignated your capital!\n");
	if (!opt_EASY_BRIDGES && IS_BRIDGE_HEAD(n))
	    bridgefall(&sect);
    }
    if (changed)
	writemap(player->cnum);
    if (breaksanct)
	bsanct();
    return RET_OK;
}
