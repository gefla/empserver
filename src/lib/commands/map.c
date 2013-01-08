/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  map.c: Display a map of sectors.
 *
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Ken Stevens, 1995
 *     Marc Olzheim, 2004
 *     Markus Armbruster, 2005-2011
 */

#include <config.h>

#include "commands.h"
#include "map.h"

int
map(void)
{
    int unit_type = EF_BAD;
    int bmap = 0;
    char *str, *prompt;
    char buf[1024];
    char prompt_buf[128];

    if (**player->argp != 'm') {
	if (**player->argp == 'b')
	    bmap = 'b';
	else if (**player->argp == 'n') {
	    unit_type = EF_NUKE;
	    if (player->argp[0][1] == 'b')
		bmap = 'b';
	    else
		bmap = 'n';
	} else {
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

    if (unit_type == EF_BAD)
	prompt = "(sects)? ";
    else {
	sprintf(prompt_buf, "(sects, %s)? ", ef_nameof(unit_type));
	prompt = prompt_buf;
    }
    str = getstarg(player->argp[1], prompt, buf);
    if (!str || !*str)
	return RET_SYN;

    if (unit_type == EF_BAD)
	unit_type = EF_SHIP;

    return do_map(bmap, unit_type, str, player->argp[2]);
}
