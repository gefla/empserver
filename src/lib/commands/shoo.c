/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shoo.c: Shoot some conquered populace or pigeons.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2004-2016
 */

#include <config.h>

#include "chance.h"
#include "commands.h"
#include "item.h"
#include "news.h"

int
shoo(void)
{
    struct sctstr sect;
    struct nstr_sect nstr;
    int seceff, nshot;
    double secstr, m;
    i_type item;
    struct ichrstr *ip;
    int targets;
    char *p;
    char prompt[128];
    char buf[1024];

    ip = whatitem(player->argp[1], "Shoot what <civ or uw> ");
    if (!ip || (ip->i_uid != I_CIVIL && ip->i_uid != I_UW))
	return RET_SYN;
    item = ip->i_uid;
    if (!snxtsct(&nstr, player->argp[2]))
	return RET_SYN;
    sprintf(prompt, "number of %s to shoot? ", ip->i_name);
    p = getstarg(player->argp[3], prompt, buf);
    if (!p || (targets = atoi(p)) <= 0)
	return RET_SYN;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	secstr = security_strength(&sect, &seceff);
	if (sect.sct_item[item] == 0 || sect.sct_item[I_CIVIL] > secstr * 10)
	    continue;
	nshot = sect.sct_item[item] > targets ? targets : sect.sct_item[item];
	if (nshot > sect.sct_mobil * 5)
	    nshot = sect.sct_mobil * 5;
	m = nshot / 5.0;
	/*
	 * Security units reduce mobility cost of shooting people by
	 * 10% per 100% unit efficiency, up to a 50% reduction.
	 */
	if (seceff > 500)
	    seceff = 500;
	m *= 1.0 - seceff / 1000.0;
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
