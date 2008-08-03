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
 *  conv.c: Convert conquered populace into uw's
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 */

/*
 * format: convert <SECTS> <NUMBER PER SECTOR>
 */

#include <config.h>

#include "commands.h"
#include "land.h"

int
conv(void)
{
    struct natstr *natp;
    struct sctstr sect;
    struct nstr_sect nstr;
    int uwtoconvert, newuw, totaluw, uw;
    int maxpop, civ, mil, adj_mob, mob;
    double security_extra = 1.0;
    struct lndstr land;
    struct nstr_item ni;


    natp = getnatp(player->cnum);
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
	mil = sect.sct_item[I_MILIT];

	/*
	 * Military units count according to the number of
	 * mil in them. (i.e. attack/defense modifier don't
	 * count.
	 */
	snxtitem_xy(&ni, EF_LAND, sect.sct_x, sect.sct_y);
	while (nxtitem(&ni, &land)) {
	    mil += land.lnd_item[I_MILIT];

	    /* Anti-terrorist units count double */
	    if (lchr[(int)land.lnd_type].l_flags & L_SECURITY) {

		/*
		 * They also increase the efficiency of
		 * the conversion process by 10% each.
		 * (but they use 10 mobility doing it)
		 */
		security_extra += .1;
		land.lnd_mobil -= 10;
		mil += land.lnd_item[I_MILIT];
	    }
	}
	/*
	 * Must have military control to convert captured civs.
	 */
	if (mil * 10 < civ)
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
