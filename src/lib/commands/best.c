/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  best.c: Show the best path between two sectors
 * 
 *  Known contributors to this file:
 *     
 */

#include <config.h>

#include "commands.h"
#include "path.h"

int
best(void)
{
    double cost;
    char *path;
    struct sctstr s1, s2;
    struct nstr_sect nstr, nstr2;
    char buf[1024];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;

    if (!snxtsct(&nstr2, player->argp[2]))
	return RET_SYN;

    while (!player->aborted && nxtsct(&nstr, &s1)) {
	if (s1.sct_own != player->cnum)
	    continue;
	snxtsct_rewind(&nstr2);
	while (!player->aborted && nxtsct(&nstr2, &s2)) {
	    if (s2.sct_own != player->cnum)
		continue;
	    path = BestLandPath(buf, &s1, &s2, &cost, MOB_MOVE);
	    if (path)
		pr("Best path from %s to %s is %s (cost %1.3f)\n",
		   xyas(s1.sct_x, s1.sct_y, player->cnum),
		   xyas(s2.sct_x, s2.sct_y, player->cnum), path, cost);
	    else
		pr("No owned path from %s to %s exists!\n",
		   xyas(s1.sct_x, s1.sct_y, player->cnum),
		   xyas(s2.sct_x, s2.sct_y, player->cnum));
	}
    }
    return 0;
}
