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

static void
update_coastal_flag(struct sctstr *sp, struct sctstr *sectp)
{
    int n;
    struct sctstr sect;

    for (n = 1; n <= 6; n++) {	/* Directions */
	getsect(sp->sct_x + diroff[n][0], sp->sct_y + diroff[n][1], &sect);
	if (sectp && sectp->sct_x == sect.sct_x &&
	    sectp->sct_y == sect.sct_y)
	    continue;
	if (sect.sct_type == SCT_WATER || sect.sct_type == SCT_BTOWER ||
	    sect.sct_type == SCT_BSPAN) {
	    if (!sp->sct_coastal) {
		sp->sct_coastal = 1;
		putsect(sp);
	    }
	    return;
	}
    }
    if (sp->sct_coastal) {
        sp->sct_coastal = 0;
	putsect(sp);
    }
}

static void
coastal_sea_to_land(struct sctstr *sp)
{
    int n;
    struct sctstr sect;

    update_coastal_flag(sp, NULL);

    for (n = 1; n <= 6; n++) {	/* Directions */
	getsect(sp->sct_x + diroff[n][0], sp->sct_y + diroff[n][1], &sect);
	update_coastal_flag(&sect, sp);
    }
}

static void
coastal_land_to_sea(struct sctstr *sp)
{
    int n;
    struct sctstr sect;

    sp->sct_coastal = 1;
    putsect(sp);

    for (n = 1; n <= 6; ++n) {	/* Directions */
	getsect(sp->sct_x + diroff[n][0], sp->sct_y + diroff[n][1], &sect);
	if (!sect.sct_coastal) {
	    sect.sct_coastal = 1;
	    putsect(&sect);
	}
    }
}

void
set_coastal(struct sctstr *sp, int des)
{
    int old_water = 0;
    int new_water = 0;

    if (sp->sct_type == SCT_WATER || sp->sct_type == SCT_BTOWER ||
	sp->sct_type == SCT_BSPAN)
	old_water = 1;

    if (des == SCT_WATER || des == SCT_BTOWER || des== SCT_BSPAN)
	new_water = 1;

    if (new_water == old_water)
	return;
    else if (new_water)
	coastal_land_to_sea(sp);
    else
	coastal_sea_to_land(sp);
}
