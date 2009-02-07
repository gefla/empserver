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
 *  nat.c: Get nation stuff
 *
 *  Known contributors to this file:
 *     Dave Pare, 1994
 */

#include <config.h>

#include "file.h"
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
natpass(int cn, char *pass)
{
    struct natstr *np;

    np = getnatp((natid)cn);
    if (np->nat_stat == STAT_VIS)
	return 1;
    if (strcmp(np->nat_pnam, pass) == 0)
	return 1;
    return 0;
}
