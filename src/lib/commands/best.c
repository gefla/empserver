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
 *  best.c: Show the best path between two sectors
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2011
 */

#include <config.h>

#include "commands.h"
#include "path.h"

int
best(void)
{
    double cost;
    struct sctstr s1, s2;
    struct nstr_sect nstr, nstr2;
    char buf[1024];
    size_t len;

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;

    if (!snxtsct(&nstr2, player->argp[2]))
	return RET_SYN;

    while (!player->aborted && nxtsct(&nstr, &s1)) {
	if (!player->owner)
	    continue;
	snxtsct_rewind(&nstr2);
	while (!player->aborted && nxtsct(&nstr2, &s2)) {
	    if (!player->owner)
		continue;
	    cost = path_find(s1.sct_x, s1.sct_y, s2.sct_x, s2.sct_y,
			     player->cnum, MOB_MOVE);
	    if (cost < 0)
		pr("No owned path from %s to %s exists!\n",
		   xyas(s1.sct_x, s1.sct_y, player->cnum),
		   xyas(s2.sct_x, s2.sct_y, player->cnum));
	    else {
		len = path_find_route(buf, sizeof(buf),
				      s1.sct_x, s1.sct_y,
				      s2.sct_x, s2.sct_y);
		pr("Best path from %s to %s is %s%s (cost %1.3f)\n",
		   xyas(s1.sct_x, s1.sct_y, player->cnum),
		   xyas(s2.sct_x, s2.sct_y, player->cnum),
		   buf, len < sizeof(buf) ? "h" : "...",
		   cost);
	    }
	}
    }
    return 0;
}
