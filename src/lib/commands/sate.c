/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2009, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  sate.c: Do satellite maps/reports.
 *
 *  Known contributors to this file:
 *     Edward M. Rynes Esq, 1988
 *     Ken Stevens, 1995
 *     Steve McClure, 1998-2000
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "optlist.h"
#include "plane.h"

int
sate(void)
{
    static int sct_shp_or_lnd[] = { EF_SECTOR, EF_SHIP, EF_LAND, EF_BAD };
    double tech;
    int pln_uid;
    struct plnstr plane;
    int type = EF_BAD;

    if (!player->argp[1] ||
	!*player->argp[1] ||
	!isdigit(*player->argp[1]) ||
	(pln_uid = atoi(player->argp[1])) < 0)
	return RET_SYN;

    if (!getplane(pln_uid, &plane)) {
	pr("No such plane\n");
	return RET_FAIL;
    }

    if (plane.pln_own != player->cnum && !player->god) {
	pr("You don't own plane #%d\n", pln_uid);
	return RET_FAIL;
    }

    if (!pln_is_in_orbit(&plane)) {
	pr("%s isn't in orbit\n", prplane(&plane));
	return RET_FAIL;
    }
    if (plane.pln_mobil < plane_mob_max) {
	pr("%s doesn't have enough mobility (needs %d)\n",
	   prplane(&plane), plane_mob_max);
	return RET_FAIL;
    }
    if (player->argp[2]) {
	type = ef_byname_from(player->argp[2], sct_shp_or_lnd);
	if (type < 0) {
	    return RET_SYN;
	}
    }

    if (plchr[(int)plane.pln_type].pl_flags & P_S)
	pr("Satellite Spy Report:\n");
    else
	pr("Satellite Map Report:\n");
    pr("%s at ", prplane(&plane));
    tech = techfact(plane.pln_tech, 20.0);
    satmap(plane.pln_x, plane.pln_y, plane.pln_effic,
	   (int)tech, plchr[(int)plane.pln_type].pl_flags, type);

    return RET_OK;
}
