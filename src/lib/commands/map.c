/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  map.c: Display a map of sectors.
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 */

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
    struct natstr *np;
    s_char *str;
    struct nstr_sect ns;
    s_char origin = '\0';
    int as_country;
    int map_flags = 0;
    int i;
    int where = 2;
    s_char what[64];
    s_char buf[1024];

    if (**player->argp != 'm') {
	if (**player->argp == 'b')
	    bmap = EF_BMAP;
	else if (**player->argp == 'n')
	    bmap = EF_NMAP;
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
		bmap = EF_BMAP;
	}
    }

    if (player->argp[1] == (s_char *)0) {
	if ((str = getstring("(sects)? ", buf)) == 0)
	    return RET_SYN;
    } else {
	str = player->argp[1];
    }

    np = getnatp(player->cnum);
    if (*str == '*') {
	sprintf(what, "%d:%d,%d:%d",
		-WORLD_X / 2, WORLD_X / 2 - 1,
		-WORLD_Y / 2, WORLD_Y / 2 - 1);
	if (!snxtsct(&ns, what))
	    return RET_FAIL;
    } else if (!snxtsct(&ns, str)) {
	i = atoi(str);
	if (unit_map(unit_type, i, &ns, &origin))
	    return RET_FAIL;
    }
    b = player->argp[2];
    while (b != (s_char *)0 && (*b)) {
	where = 3;
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
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    where = 2;
	    break;
	case 't':
	    if (!bmap)
		goto bad_flag;
	    bmap = EF_MAP;
	    *(b + 1) = 0;
	    break;
	case 'r':
	    if (!bmap)
		goto bad_flag;
	    bmap = EF_MAP + EF_BMAP;
	    *(b + 1) = 0;
	    break;
	default:
	bad_flag:
	    pr("Bad flag %c!\n", *b);
	    break;
	}
	b++;
    }

    as_country = player->cnum;
    if (player->god) {
	if (player->argp[where] != (s_char *)0) {
	    as_country = atoi(player->argp[where]);
	    if ((as_country < 0) || (as_country > MAXNOC)) {
		as_country = player->cnum;
	    }
	}
    }

    return draw_map(bmap, origin, map_flags, &ns, as_country);
}
