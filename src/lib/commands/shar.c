/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2015, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  shar.c: share a bmap with a friend
 *
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 */

#include <config.h>

#include "commands.h"
#include "map.h"

int
shar(void)
{
    int to;
    struct nstr_sect ns;
    char des = 0;
    int n;

    to = natarg(player->argp[1], "Share bmap with which country? ");
    if (to < 0)
	return RET_SYN;

    if (relations_with(to, player->cnum) < FRIENDLY) {
	pr("%s does not have friendly relations towards you\n", cname(to));
	return RET_FAIL;
    }

    if (!snxtsct(&ns, player->argp[2]))
	return RET_SYN;

    if (player->argp[3] && *player->argp[3]) {
	if (sct_typematch(player->argp[3]) < 0)
	    return RET_SYN;
	else
	    des = *player->argp[3];
    }

    if (!bmaps_intersect(player->cnum, to)) {
	pr("Your bmap does not intersect %s's bmap.\n", cname(to));
	return RET_FAIL;
    }

    n = share_bmap(player->cnum, to, &ns, des, cname(player->cnum));
    pr("%d designations transmitted\n", n);

    return RET_OK;
}
