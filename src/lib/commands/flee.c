/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2006, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  flee.c: Add ships to a fleet
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "ship.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

int
flee(void)
{
    struct shpstr ship;
    int count;
    s_char *cp;
    s_char c;
    struct nstr_item nstr;
    struct nstr_item ni;
    struct shpstr ship2;
    s_char buf[1024];

    cp = getstarg(player->argp[1], "fleet? ", buf);
    if (cp == 0)
	return RET_SYN;
    c = *cp;
    if (!isalpha(c) && c != '~') {
	pr("Specify fleet, (1 alpha char or '~')\n");
	return RET_SYN;
    }
    if (c == '~')
	c = ' ';
    if (!snxtitem(&nstr, EF_SHIP, player->argp[2]))
	return RET_SYN;
    count = 0;
    while (nxtitem(&nstr, &ship)) {
	if (!player->owner)
	    continue;
	if (ship.shp_fleet == c)
	    continue;
	ship.shp_rflags &= ~RET_GROUP;
	snxtitem_group(&ni, EF_SHIP, c);
	while (nxtitem(&ni, &ship2)) {
	    if ((ship2.shp_rflags & RET_GROUP) == 0)
		continue;
	    if (ship2.shp_x == ship.shp_x && ship2.shp_y == ship.shp_y) {
		memcpy(ship.shp_rpath, ship2.shp_rpath,
		       sizeof(ship.shp_rpath));
		ship.shp_rflags = ship2.shp_rflags;
		break;
	    }
	}
	ship.shp_fleet = c;
	putship(ship.shp_uid, &ship);
	count++;
    }
    pr("%d ship%s added to fleet `%c'\n", count, splur(count), c);
    return RET_OK;
}
