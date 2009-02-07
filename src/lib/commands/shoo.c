/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shoo.c: Shoot some conquered populace or pigeons.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

#include <config.h>

#include "commands.h"
#include "item.h"
#include "land.h"
#include "news.h"
#include "path.h"

int
shoo(void)
{
    struct sctstr sect;
    struct nstr_sect nstr;
    struct nstr_item ni;
    int nshot;
    double m;
    i_type item;
    struct ichrstr *ip;
    struct lndstr land;
    int targets;
    char *p;
    int mil, nsec;
    char prompt[128];
    char buf[1024];

    ip = whatitem(player->argp[1], "Shoot what <civ or uw> ");
    if (ip == 0 || (ip->i_uid != I_CIVIL && ip->i_uid != I_UW))
	return RET_SYN;
    item = ip->i_uid;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    sprintf(prompt, "number of %s to shoot? ", ip->i_name);
    p = getstarg(player->argp[3], prompt, buf);
    if (p == 0 || (targets = atoi(p)) <= 0)
	return RET_SYN;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	mil = sect.sct_item[I_MILIT];
	nsec = 0;
	snxtitem_xy(&ni, EF_LAND, sect.sct_x, sect.sct_y);
	while (nxtitem(&ni, &land)) {
	    mil += land.lnd_item[I_MILIT];

	    if (lchr[(int)land.lnd_type].l_flags & L_SECURITY) {
		mil += land.lnd_item[I_MILIT];
		nsec++;
	    }
	}

	if (sect.sct_item[item] == 0 || sect.sct_item[I_CIVIL] > mil * 10)
	    continue;
	nshot = sect.sct_item[item] > targets ? targets : sect.sct_item[item];
	if (nshot > sect.sct_mobil * 5)
	    nshot = sect.sct_mobil * 5;
	m = nshot / 5.0;
	/*
	 * Each security unit lowers the cost of
	 * shooting a person by 10%. However, you
	 * can't go lower than 50% of normal cost
	 */
	if (nsec > 5)
	    nsec = 5;
	m *= 1.0 - nsec * 0.1;
	if (nshot <= 0)
	    continue;
	if (m < 0)
	    m = 0;
	if (m > sect.sct_mobil)
	    m = sect.sct_mobil;
	sect.sct_mobil -= roundavg(m);
	sect.sct_item[item] -= nshot;
	pr("BANG!! (thump) %d %s shot in %s!\n",
	   nshot, ip->i_name, xyas(sect.sct_x, sect.sct_y, player->cnum));
	if (chance(nshot / 100.0))
	    nreport(player->cnum, N_SHOOT_CIV, sect.sct_oldown, 1);
	if (sect.sct_item[item] <= 0 && item == I_CIVIL
	    && (sect.sct_own != sect.sct_oldown)) {
	    sect.sct_oldown = sect.sct_own;
	    pr("  %s is now completely yours\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	}
	putsect(&sect);
    }
    return RET_OK;
}
