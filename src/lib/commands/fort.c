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
 *  fort.c: Increase the fortification value of land units
 * 
 *  Known contributors to this file:
 *    Steve McClure, 1999
 *     
 */

#include "misc.h"
#include "player.h"
#include "var.h"
#include "xy.h"
#include "sect.h"
#include "land.h"
#include "nat.h"
#include "nsc.h"
#include "deity.h"
#include "file.h"
#include "commands.h"

int
fort(void)
{
    int nunits;
    struct nstr_item ni;
    struct lndstr land;
    int fort_amt, hard_amt;
    s_char *p;
    extern int land_mob_max;
    s_char buf[1024];

    if (!snxtitem(&ni, EF_LAND, player->argp[1]))
	return RET_SYN;
    p = getstarg(player->argp[2], "Amount: ", buf);
    if (p == 0 || *p == 0)
	return RET_SYN;
    fort_amt = atoi(p);
    if (fort_amt > land_mob_max)
	fort_amt = land_mob_max;
    nunits = 0;
    while (nxtitem(&ni, (s_char *)&land)) {
	if (!player->owner || land.lnd_own == 0)
	    continue;
	if (land.lnd_type < 0 || land.lnd_type > lnd_maxno) {
	    pr("bad unit type %d (#%d)\n", land.lnd_type, ni.cur);
	    continue;
	}

	getland(land.lnd_uid, &land);
	if (land.lnd_mobil < 0)
	    continue;
	if (land.lnd_ship >= 0) {
	    pr("%s is on a ship and can't be fortified\n", prland(&land));
	    continue;
	}

	hard_amt = fort_amt;

	/* This is use for things like "fort * -67" which will
	   use all mobility down to leaving 67 left. */
	if (hard_amt < 0) {
	    hard_amt = land.lnd_mobil + hard_amt;
	    if (hard_amt < 0)
		continue;
	}

	nunits++;

	lnd_fortify (&land, hard_amt);

	pr("%s hardened to %d\n", prland(&land), land.lnd_harden);

	putland(land.lnd_uid, &land);
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
