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
 *  foll.c: Set leader of a set of ships
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
#include "nat.h"
#include "commands.h"
#include "optlist.h"

int
foll(void)
{
    struct shpstr ship;
    s_char *cp;
    int good, leader, count = 0;
    coord x, y;
    struct nstr_item nstr;
    s_char buf[1024];

    if (!opt_SAIL) {
	pr("The SAIL option is not enabled, so this command is not valid.\n");
	return RET_FAIL;
    }
    if (!snxtitem(&nstr, EF_SHIP, player->argp[1]))
	return RET_SYN;
    cp = getstarg(player->argp[2], "leader?", buf);
    if (cp == 0)
	cp = "";
    good = sscanf(cp, "%d", &leader);
    if (!good)
	return RET_SYN;
    getship(leader, &ship);
    if (ship.shp_own != player->cnum &&
	getrel(getnatp(ship.shp_own), player->cnum) < FRIENDLY) {
	pr("That ship won't let you follow.\n");
	return RET_FAIL;
    }
    x = ship.shp_x;
    y = ship.shp_y;
    while (nxtitem(&nstr, &ship)) {
	if (!player->owner)
	    continue;
	if (ship.shp_x != x || ship.shp_y != y) {
	    pr("Ship #%d not in same sector as #%d\n", ship.shp_uid,
	       leader);
	    continue;
	}
	if (ship.shp_uid == leader) {
	    pr("Ship #%d can't follow itself!\n", leader);
	    continue;
	}
	if ((ship.shp_autonav & AN_AUTONAV)
	    && !(ship.shp_autonav & AN_STANDBY)) {
	    pr("Ship #%d has other orders!\n", ship.shp_uid);
	    continue;
	}
	count++;
	ship.shp_mission = 0;
	*ship.shp_path = 'f';
	ship.shp_path[1] = 0;
	ship.shp_follow = leader;
	pr("Ship #%d follows #%d.\n", ship.shp_uid, leader);
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
