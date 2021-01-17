/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2021, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  conv.c: Convert conquered populace into uw's
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Markus Armbruster, 2004-2016
 */

/*
 * format: convert <SECTS> <NUMBER PER SECTOR>
 */

#include <config.h>

#include "chance.h"
#include "commands.h"

int
conv(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    int uwtoconvert, newuw, totaluw, uw;
    int maxpop, civ, seceff, adj_mob, mob;
    double secstr;
    double security_extra = 1.0;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    uwtoconvert = onearg(player->argp[2], "Number to convert: ");
    if (uwtoconvert < 0)
	return RET_SYN;

    totaluw = 0;
    while (nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	if (sect.sct_oldown == player->cnum)
	    continue;
	natp = getnatp(sect.sct_own);
	maxpop = max_pop(natp->nat_level[NAT_RLEV], &sect);
	civ = sect.sct_item[I_CIVIL];
	secstr = security_strength(&sect, &seceff);
	/*
	 * Must have military control to convert captured civs.
	 */
	if (secstr * 10 < civ)
	    continue;
	newuw = civ;
	if (newuw > uwtoconvert)
	    newuw = uwtoconvert;
	uw = sect.sct_item[I_UW];
	if (newuw > maxpop - uw)
	    newuw = maxpop - uw;
	if (newuw <= 0)
	    continue;
	/*
	 * So entire civilian populations don't disappear immediately
	 * into re-education camps, charge a healthy mobility cost for
	 * conversions.
	 */
	mob = sect.sct_mobil * 5;

	/* security troops make conversion more effective */
	security_extra = 1.0 + seceff / 1000.0;
	adj_mob = ldround(((double)mob * security_extra), 1);

	if (adj_mob < newuw)
	    newuw = adj_mob;
	if (newuw <= 0)
	    continue;
	if (player->dolcost + newuw * 1.5 > natp->nat_money) {
	    pr("You can't afford to convert %d civilians in %s!\n",
	       newuw, xyas(sect.sct_x, sect.sct_y, player->cnum));
	    break;
	}
	player->btused += (newuw - 1) / 100 + 1;
	player->dolcost += newuw * 1.5;
	if (newuw < mob)
	    mob = newuw;
	sect.sct_item[I_UW] = newuw + uw;
	civ -= newuw;
	sect.sct_item[I_CIVIL] = civ;
	mob = roundavg(mob * 0.2);
	if (mob > sect.sct_mobil)
	    mob = sect.sct_mobil;
	sect.sct_mobil -= mob;
	pr("%3d conquered civilians converted in %s (%d)\n",
	   newuw, xyas(sect.sct_x, sect.sct_y, player->cnum), uw + newuw);
	if (civ == 0) {
	    sect.sct_oldown = sect.sct_own;
	    pr("%s is now completely yours.\n",
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	}
	putsect(&sect);
	totaluw += newuw;
    }
    pr("Total civilians converted: %d\n", totaluw);
    pr("Paperwork at conversion places ... %d\n", player->btused);
    return RET_OK;
}
