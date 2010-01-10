/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2010, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  bigmap.c:  Updates a player's sector map using sector offsets and
 *             simple overwrite rules.
 *
 *  Known contributors to this file:
 *
 */

#include <config.h>

#include "file.h"
#include "map.h"
#include "sect.h"

/* values of only_bmap:
 * 0 = update both maps
 * 1 = update only bmap
 * 2 = update only bmap with force
 */

static int do_map_set(char *map, coord x, coord y, char t, int force);

int
map_set(natid cnum, coord x, coord y, char t, int only_bmap)
{
    int set = 0;

    if (!t)
	return 0;
    if (!only_bmap)
	set |= do_map_set(ef_ptr(EF_MAP, cnum), x, y, t, 0);
    set |= do_map_set(ef_ptr(EF_BMAP, cnum), x, y, t, only_bmap > 1);

    return set;
}


static int
do_map_set(char *map, coord x, coord y, char t, int force)
{
    int id;

    if ((id = sctoff(x, y)) < 0)
	return 0;

    if (((map[id] == 'x') || (map[id] == 'X')) && !force)
	return 0;

    if (t == '?') {
	switch (map[id]) {
	case '.':
	case '-':
	case ' ':
	case 0:
	    map[id] = t;
	    break;
	default:
	    break;
	}
    } else {
	map[id] = t;
    }
    return 1;
}

void
writebmap(natid cnum)
{
    ef_write(EF_BMAP, cnum, ef_ptr(EF_BMAP, cnum));
}

void
writemap(natid cnum)
{
    ef_write(EF_MAP, cnum, ef_ptr(EF_MAP, cnum));
    ef_write(EF_BMAP, cnum, ef_ptr(EF_BMAP, cnum));
}
