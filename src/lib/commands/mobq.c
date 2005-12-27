/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  mobq.c: Set the sailing mobility quota for a ship
 * 
 *  Known contributors to this file:
 *     Robert Forsman
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "ship.h"
#include "xy.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"
#include "optlist.h"


int
mobq(void)
{
    struct shpstr ship;
    s_char *cp, *oldmq;
    int good, mobquota, count = 0;
    struct nstr_item nstr;
    s_char buf[1024];

    if (!opt_SAIL) {
	pr("The SAIL option is not enabled, so this command is not valid.\n");
	return RET_FAIL;
    }
    if (!snxtitem(&nstr, EF_SHIP, player->argp[1]))
	return RET_SYN;
    oldmq = player->argp[2];
    if (oldmq) {
	good = sscanf(oldmq, "%d", &mobquota);
	if (!good)
	    return RET_SYN;
	if (mobquota < 0 || mobquota > ship_mob_max) {
	    pr("Bad mobility quota value %d.\n", mobquota);
	    return RET_SYN;
	}
	if (mobquota + (ship_mob_scale * (float)etu_per_update) >
	    ship_mob_max) {
	    pr("warning: %d less than optimal\n", mobquota);
	}
    }
    while (nxtitem(&nstr, &ship)) {
	if (!player->owner)
	    continue;
	if (!oldmq)
	    pr("Ship #%d at %s.  Old value %d.\n", ship.shp_uid,
	       xyas(ship.shp_x, ship.shp_y, player->cnum),
	       ship.shp_mobquota);
	cp = getstarg(player->argp[2], "mobility quota?", buf);
	if (!cp)
	    return RET_SYN;
	if (!check_ship_ok(&ship))
	    continue;
	good = sscanf(cp, "%d", &mobquota);
	if (!good) {
	    pr("Huh?\n");
	    continue;
	}
	if (!oldmq) {
	    if (mobquota < 0 || mobquota > ship_mob_max) {
		pr("Bad mobility quota value %d.\n", mobquota);
		continue;
	    }
	    if (mobquota + (ship_mob_scale * (float)etu_per_update) >
		ship_mob_max) {
		pr("warning: %d less than optimal\n", mobquota);
	    }
	}
	ship.shp_mobquota = mobquota;
	count++;
	putship(ship.shp_uid, &ship);
    }
    if (count == 0) {
	if (player->argp[1])
	    pr("%s: No ship(s)\n", player->argp[1]);
	else
	    pr("%s: No ship(s)\n", "");
	return RET_FAIL;
    } else
	pr("%d ship%s\n", count, splur(count));
    return RET_OK;
}
