/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  improve.c: Improve the infrastructure of a sector
 *
 *  Known contributors to this file:
 *     Steve McClure, 1996-2000
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "path.h"

int
improve(void)
{
    struct sctstr sect;
    int nsect;
    struct nstr_sect nstr;
    char *p;
    char buf[1024];
    char prompt[128];
    int type;
    int value;
    int ovalue;
    int maxup;
    struct natstr *natp;
    int lneeded;
    int hneeded;
    int mneeded;
    int dneeded;
    int wanted;

    p = getstarg(player->argp[1],
		 "Improve what ('road', 'rail' or 'defense')? ",
		 buf);
    if (!p || !*p)
	return RET_SYN;
    if (!strncmp(p, "ro", 2))
	type = INT_ROAD;
    else if (!strncmp(p, "ra", 2))
	type = INT_RAIL;
    else if (!strncmp(p, "de", 2)) {
	type = INT_DEF;
    } else
	return RET_SYN;

    if (!intrchr[type].in_enable) {
	pr("%s improvement is disabled.\n", intrchr[type].in_name);
	return RET_FAIL;
    }

    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    prdate();
    nsect = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (type == INT_ROAD)
	    value = sect.sct_road;
	else if (type == INT_RAIL)
	    value = sect.sct_rail;
	else /* type == INT_DEF */
	    value = sect.sct_defense;
	sprintf(prompt, "Sector %s has a %s of %d%%.  Improve how much? ",
		xyas(sect.sct_x, sect.sct_y, player->cnum),
		intrchr[type].in_name, value);
	p = getstarg(player->argp[3], prompt, buf);
	if (!p || !*p)
	    continue;
	if (!check_sect_ok(&sect))
	    continue;
	maxup = 100 - value;
	wanted = atoi(p);
	if (wanted < 0)
	    continue;
	if (wanted < maxup)
	    maxup = wanted;
	if (!maxup)
	    continue;
	lneeded = intrchr[type].in_lcms * maxup;
	if (sect.sct_item[I_LCM] < lneeded) {
	    lneeded = sect.sct_item[I_LCM];
	    maxup = lneeded / intrchr[type].in_lcms;
	    if (maxup <= 0) {
		pr("Not enough lcms in %s\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    }
	}
	hneeded = intrchr[type].in_hcms * maxup;
	if (sect.sct_item[I_HCM] < hneeded) {
	    hneeded = sect.sct_item[I_HCM];
	    maxup = hneeded / intrchr[type].in_hcms;
	    if (maxup <= 0) {
		pr("Not enough hcms in %s\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    }
	}
	mneeded = intrchr[type].in_mcost * maxup;
	if ((sect.sct_mobil - 1) < mneeded) {
	    mneeded = sect.sct_mobil - 1;
	    if (mneeded < 0)
		mneeded = 0;
	    maxup = mneeded / intrchr[type].in_mcost;
	    if (maxup <= 0) {
		pr("Not enough mobility in %s\n",
		   xyas(sect.sct_x, sect.sct_y, player->cnum));
		continue;
	    }
	}
	dneeded = intrchr[type].in_dcost * maxup;
	natp = getnatp(player->cnum);
	if (player->dolcost + dneeded > natp->nat_money) {
	    pr("Not enough money left to improve %s by %d%%\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum), maxup);
	    break;
	}
	lneeded = intrchr[type].in_lcms * maxup;
	hneeded = intrchr[type].in_hcms * maxup;
	mneeded = intrchr[type].in_mcost * maxup;
	dneeded = intrchr[type].in_dcost * maxup;
	player->dolcost += dneeded;
	sect.sct_item[I_LCM] -= lneeded;
	sect.sct_item[I_HCM] -= hneeded;
	sect.sct_mobil -= mneeded;
	ovalue = value;
	value += maxup;
	if (CANT_HAPPEN(value > 100))
	    value = 100;
	pr("Sector %s %s increased from %d%% to %d%%\n",
	   xyas(sect.sct_x, sect.sct_y, player->cnum),
	   intrchr[type].in_name, ovalue, value);
	if (type == INT_ROAD)
	    sect.sct_road = value;
	else if (type == INT_RAIL)
	    sect.sct_rail = value;
	else if (type == INT_DEF)
	    sect.sct_defense = value;
	putsect(&sect);
	nsect++;
    }
    if (nsect == 0) {
	if (player->argp[2])
	    pr("%s: No sector(s)\n", player->argp[1]);
	else
	    pr("%s: No sector(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d sector%s\n", nsect, splur(nsect));
    return 0;
}
