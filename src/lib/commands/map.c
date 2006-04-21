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
 *  See files README, COPYING and CREDITS in the root of the source
 *  tree for related information and legal notices.  It is expected
 *  that future projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  map.c: Display a map of sectors.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "misc.h"
#include "nat.h"
#include "file.h"
#include "player.h"
#include "map.h"
#include "ship.h"
#include "land.h"
#include "xy.h"
#include "nsc.h"
#include "commands.h"
#include "optlist.h"

int
map(void)
{
    register s_char *b;
    int unit_type = 0;
    int bmap = 0;
    struct nstr_sect ns;
    s_char origin = '\0';
    int map_flags = 0;

    if (**player->argp != 'm') {
	if (**player->argp == 'b')
	    bmap = 'b';
	else if (**player->argp == 'n')
	    bmap = 'n';
	else {
	    if (**player->argp == 'l')
		unit_type = EF_LAND;
	    else if (**player->argp == 'p')
		unit_type = EF_PLANE;
	    else if (**player->argp == 's')
		unit_type = EF_SHIP;
	    else {
		logerror("map: bad player->argp[0]");
		return RET_SYN;
	    }
	    if (player->argp[0][1] == 'b')
		bmap = 'b';
	}
    }

    if (!snxtsct(&ns, player->argp[1])) {
	if (unit_map(unit_type, atoi(player->argp[1]), &ns, &origin))
	    return RET_FAIL;
    }
    for (b = player->argp[2]; b && *b; b++) {
	switch (*b) {
	case 's':
	case 'S':
	    map_flags |= MAP_SHIP;
	    break;
	case 'l':
	case 'L':
	    map_flags |= MAP_LAND;
	    break;
	case 'p':
	case 'P':
	    map_flags |= MAP_PLANE;
	    break;
	case 'h':
	case 'H':
	    map_flags |= MAP_HIGH;
	    break;
	case '*':
	    map_flags |= MAP_ALL;
	    break;
	case 't':
	    if (bmap != 'b')
		goto bad_flag;
	    bmap = 't';
	    *(b + 1) = 0;
	    break;
	case 'r':
	    if (bmap != 'b')
		goto bad_flag;
	    bmap = 'r';
	    *(b + 1) = 0;
	    break;
	default:
	bad_flag:
	    pr("Bad flag %c!\n", *b);
	    break;
	}
    }

    return draw_map(bmap, origin, map_flags, &ns);
}
