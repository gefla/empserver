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
 *  desi.c: Redesignate sectors
 * 
 *  Known contributors to this file:
 *      Steve McClure, 1998-2000
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "sect.h"
#include "item.h"
#include "xy.h"
#include "path.h"
#include "nsc.h"
#include "file.h"
#include "nat.h"
#include "optlist.h"
#include "commands.h"

static long do_desi(struct natstr *natp, s_char *sects, s_char *deschar,
		    long int cash, int for_real);

int
desi(void)
{
    long cash;
    long cost;
    struct natstr *natp;

    natp = getnatp(player->cnum);
    cash = natp->nat_money;
    if (player->argp[2]) {
	cost = do_desi(natp, player->argp[1], player->argp[2], cash, 0);
	if (cost < 0)
	    return (int)(-cost);
	if (chkmoney(cost, cash, player->argp[3]))
	    return RET_SYN;
    }
    return (int)do_desi(natp, player->argp[1], player->argp[2], cash, 1);
}

static long
do_desi(struct natstr *natp, s_char *sects, s_char *deschar, long int cash,
	int for_real)
{
    register int n;
    s_char *p;
    int breaksanct;
    int cap_x;
    int cap_y;
    int des;
    struct nstr_sect nstr;
    struct sctstr sect;
    struct sctstr check;
    s_char prompt[128];
    s_char buf[1024];
    long cost = 0;
    int changed = 0;
    int warned = 0;

    breaksanct = 0;
    if (!snxtsct(&nstr, sects)) {
	if (for_real)
	    return (long)RET_SYN;
	else
	    return (long)-RET_SYN;
    }
    cap_x = natp->nat_xcap;
    cap_y = natp->nat_ycap;
    while (!player->aborted && nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (!player->god) {
	    if (sect.sct_type == SCT_MOUNT ||
		sect.sct_type == SCT_BTOWER ||
		sect.sct_type == SCT_BSPAN ||
		sect.sct_type == SCT_WASTE ||
		sect.sct_type == SCT_PLAINS ||
		(opt_NO_LCMS && sect.sct_type == SCT_LIGHT) ||
		(opt_NO_HCMS && sect.sct_type == SCT_HEAVY) ||
		(opt_NO_OIL && sect.sct_type == SCT_OIL) ||
		(opt_NO_OIL && sect.sct_type == SCT_REFINE))
		continue;
	}
	sprintf(prompt, "%s %d%% %s  desig? ",
		xyas(sect.sct_x, sect.sct_y, player->cnum),
		sect.sct_effic, dchr[sect.sct_type].d_name);
	if ((p = getstarg(deschar, prompt, buf)) == 0)
	    continue;

	if (!check_sect_ok(&sect))
	    continue;

	des = typematch(p, EF_SECTOR);
	if (des < 0 || (((des == SCT_BSPAN) || (des == SCT_BTOWER)) &&
			!player->god)) {
	    pr("See \"info Sector-types\"\n");
	    if (for_real)
		return (long)RET_FAIL;
	    else
		return (long)-RET_FAIL;
	}
	if (!player->god) {
	    if (des == SCT_WATER || des == SCT_MOUNT ||
		des == SCT_SANCT || des == SCT_PLAINS ||
		(opt_NO_LCMS && des == SCT_LIGHT) ||
		(opt_NO_HCMS && des == SCT_HEAVY) ||
		(opt_NO_OIL && des == SCT_OIL) ||
		(opt_NO_OIL && des == SCT_REFINE)) {
		pr("Only %s can make a %s!\n", cname(0), dchr[des].d_name);
		continue;
	    }
	    if (des == SCT_WASTE) {
		pr("Only a nuclear device (or %s) can make a %s!\n",
		   cname(0), dchr[des].d_name);
		continue;
	    }
	}
	if (sect.sct_type == des && sect.sct_newtype == des)
	    continue;
	if (sect.sct_type == SCT_SANCT)
	    breaksanct++;
	if (des == SCT_HARBR) {
	    for (n = 1; n <= 6; n++) {
		getsect(nstr.x + diroff[n][0],
			nstr.y + diroff[n][1], &check);
		if (check.sct_type == SCT_WATER)
		    break;
		if (check.sct_type == SCT_BSPAN)
		    break;
		if (check.sct_type == SCT_BTOWER)
		    break;
	    }
	    if (n > 6) {
		if (for_real)
		    pr("%s does not border on water.\n",
		       xyas(nstr.x, nstr.y, player->cnum));
		if (player->god) {
		    if (for_real)
			pr("But if it's what you want ...\n");
		} else
		    continue;
	    }
	}
	if (des == SCT_BHEAD) {
	    for (n = 1; n <= 6; n++) {
		getsect(nstr.x + diroff[n][0],
			nstr.y + diroff[n][1], &check);
		if (check.sct_type == SCT_WATER)
		    break;
		if (check.sct_type == SCT_BSPAN)
		    break;
		if (check.sct_type == SCT_BTOWER)
		    break;
	    }
	    if (n > 6) {
		if (for_real)
		    pr("%s does not border on water.\n",
		       xyas(nstr.x, nstr.y, player->cnum));
		if (player->god) {
		    if (for_real)
			pr("But if it's what you want...\n");
		} else
		    continue;
	    }
	}
	if (sect.sct_type == SCT_SANCT && !player->god)
	    continue;
	n = sect.sct_type;
	if ((sect.sct_newtype != des) && (sect.sct_type != des)
	    && dchr[des].d_cost) {
	    if (for_real) {
		if (check_cost(!deschar, dchr[des].d_cost, cash, &warned,
			       player->argp[3]))
		    break;
	    } else {
		cost += dchr[des].d_cost;
		continue;
	    }
	}
	if (sect.sct_effic < 5 || player->god) {
	    sect.sct_type = des;
	    sect.sct_effic = 0;
	    changed += map_set(player->cnum, sect.sct_x, sect.sct_y,
			       dchr[des].d_mnem, 0);
	}
	sect.sct_newtype = des;
	putsect(&sect);
	if (sect.sct_x == cap_x && sect.sct_y == cap_y
	    && des != SCT_CAPIT && des != SCT_SANCT && des != SCT_MOUNT
	    && for_real)
	    pr("You have redesignated your capital!\n");
	if (opt_EASY_BRIDGES == 0) {	/* may cause a bridge fall */
	    if (n != SCT_BHEAD)
		continue;
	    bridgefall(&sect, 0);
	}
    }
    if (for_real) {
	if (changed)
	    writemap(player->cnum);
	if (breaksanct)
	    bsanct();
	return (long)RET_OK;
    } else {
	return cost;
    }
}
