/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2020, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  nat.c: Get nation stuff
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <config.h>

#include "nat.h"
#include "prototypes.h"

int
natbyname(char *name, natid *result)
{
    struct natstr *np;
    int i;

    for (i = 0; NULL != (np = getnatp(i)); i++) {
	if (np->nat_stat != STAT_UNUSED && !strcmp(np->nat_cnam, name)) {
	    *result = i;
	    return 0;
	}
    }
    *result = NATID_BAD;
    return -1;
}

int
natpass(natid cn, char *pass)
{
    struct natstr *np;

    np = getnatp(cn);
    if (np->nat_stat == STAT_VIS)
	return 1;
    if (strcmp(np->nat_pnam, pass) == 0)
	return 1;
    return 0;
}
