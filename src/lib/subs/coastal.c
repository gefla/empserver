/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  coastal.c: Routines to calculate the coastal flag
 *
 *  Known contributors to this file:
 *     Ron Koenderink, 2005
 */

#include <config.h>

#include "file.h"
#include "path.h"
#include "prototypes.h"
#include "sect.h"

static int
update_coastal_flag(coord x, coord y, coord ign_x, coord ign_y)
{
    int n;
    struct sctstr sect;

    for (n = 1; n <= 6; n++) {	/* Directions */
	getsect(x + diroff[n][0], y + diroff[n][1], &sect);
	if (sect.sct_x == ign_x && sect.sct_y == ign_y)
	    continue;
	if (sect.sct_type == SCT_WATER || sect.sct_type == SCT_BTOWER ||
	    sect.sct_type == SCT_BSPAN)
	    return 1;
    }

    return 0;
}

static int
coastal_sea_to_land(coord x, coord y)
{
    int n, c;
    struct sctstr sect;

    for (n = 1; n <= 6; n++) {	/* Directions */
	getsect(x + diroff[n][0], y + diroff[n][1], &sect);
	c = update_coastal_flag(sect.sct_x, sect.sct_y, x, y);
	if (!sect.sct_coastal != !c) {
	    sect.sct_coastal = c;
	    putsect(&sect);
	}
    }

    return update_coastal_flag(x, y, x, y);
}

static int
coastal_land_to_sea(coord x, coord y)
{
    int n;
    struct sctstr sect;

    for (n = 1; n <= 6; ++n) {	/* Directions */
	getsect(x + diroff[n][0], y + diroff[n][1], &sect);
	if (!sect.sct_coastal) {
	    sect.sct_coastal = 1;
	    putsect(&sect);
	}
    }

    return 1;
}

/*
 * Compute coastal flags for a change of SP from OLDDES to NEWDES.
 * Update adjacent sectors, but don't touch SP.
 * Return new coastal flag for SP.
 */
void
set_coastal(struct sctstr *sp, int olddes, int newdes)
{
    int old_water = olddes == SCT_WATER
	|| olddes == SCT_BTOWER || olddes == SCT_BSPAN;
    int new_water = newdes == SCT_WATER
	|| newdes == SCT_BTOWER || newdes == SCT_BSPAN;

    if (new_water != old_water)
	sp->sct_coastal = new_water
	    ? coastal_land_to_sea(sp->sct_x, sp->sct_y)
	    : coastal_sea_to_land(sp->sct_x, sp->sct_y);
}
