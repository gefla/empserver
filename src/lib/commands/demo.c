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
 *  demo.c: De-mobilize "n" military in a given sector
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "commands.h"

/*
 * format: demo <SECTS> number
 *
 */

int
demo(void)
{
    struct natstr *natp;
    int number;
    char *p;
    char buf[1024];
    struct nstr_sect nstr;
    struct sctstr sect;
    int mil_demob;
    int mil;
    int civ;
    int deltamil;
    int reserves;

    natp = getnatp(player->cnum);
    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    if (!(p = getstarg(player->argp[2], "Number to de-mobilize : ", buf)))
	return RET_SYN;
    number = atoi(p);
    if (!(p = getstarg(player->argp[3],
		       "New civilians on active reserve? (y/n) ", buf)))
	return RET_SYN;
    if (*p != 'y' && *p != 'n')
	return RET_SYN;

    mil_demob = 0;
    reserves = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner || sect.sct_effic < 60)
	    continue;
	if ((mil = sect.sct_item[I_MILIT]) == 0)
	    continue;
	if (sect.sct_own != sect.sct_oldown)
	    continue;
	civ = sect.sct_item[I_CIVIL];
	deltamil = number < 0 ? mil + number : MIN(mil, number);
	if (deltamil <= 0)
	    continue;
	if (deltamil > ITEM_MAX - civ)
	    deltamil = ITEM_MAX - civ;
	if (player->dolcost + deltamil * 5 > natp->nat_money) {
	    pr("You can't afford to demobilize %d military in %s!\n",
	       deltamil, xyas(sect.sct_x, sect.sct_y, player->cnum));
	    break;
	}
	player->dolcost += deltamil * 5;
	civ += deltamil;
	mil -= deltamil;
	mil_demob += deltamil;
	pr("%d demobilized in %s (%d mil left)\n",
	   deltamil, xyas(sect.sct_x, sect.sct_y, player->cnum), mil);
	if (*p == 'y')
	    reserves += deltamil;
	sect.sct_item[I_MILIT] = mil;
	sect.sct_item[I_CIVIL] = civ;
	putsect(&sect);
    }
    if (!mil_demob) {
	pr("No eligible sectors/military for demobilization\n");
	return RET_FAIL;
    }
    pr("Total new civilians : %d\n", mil_demob);
    if (*p == 'y')
	pr("Military reserve stands at %ld (up %d)\n",
	   natp->nat_reserve + reserves, reserves);
    if (reserves > 0) {
	natp->nat_reserve += reserves;
	putnat(natp);
    }
    return RET_OK;
}
