/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2007, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  name.c: Name a ship
 * 
 *  Known contributors to this file:
 *     Jeff Bailey
 */

#include <config.h>

#include "commands.h"
#include "optlist.h"
#include "path.h"
#include "ship.h"

/*
 * name <SHIP> <NAME>
 */
int
name(void)
{
    struct shpstr ship;
    char *p;
    struct nstr_item nb;
    char buf[1024];

    if (!snxtitem(&nb, EF_SHIP, player->argp[1]))
	return RET_SYN;
    while (nxtitem(&nb, &ship)) {
	if (!player->owner)
	    continue;
	p = getstarg(player->argp[2], "Name? ", buf);
	if (!check_ship_ok(&ship))
	    return RET_FAIL;
	if (p == 0 || *p == 0)
	    return RET_SYN;
	if (!strcmp(p, "~")) {
	    ship.shp_name[0] = 0;
	} else {
	    strncpy(ship.shp_name, p, MAXSHPNAMLEN - 1);
	    ship.shp_name[MAXSHPNAMLEN - 1] = 0;
	}
	putship(ship.shp_uid, &ship);
    }

    return RET_OK;
}
