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
 *  dist.c: Name distribution sector for a given range of sectors
 * 
 *  Known contributors to this file:
 *     Dave Pare, 1986
 *     Thomas Ruschak, 1993 (rewritten) 
 *     Steve McClure, 1998
 */

#include <config.h>

#include "misc.h"
#include "player.h"
#include "xy.h"
#include "sect.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "path.h"
#include "commands.h"

/*
 * distribute <SECT> <DISTSECT|.|h>
 */
int
dist(void)
{
    struct sctstr sect, dsect, tsect;
    struct nstr_sect nstr;
    char *path, *p;
    double move_cost = 0.0;
    coord dstx, dsty;
    char buf[1024];

    if (!snxtsct(&nstr, player->argp[1]))
	return RET_SYN;
    while (!player->aborted && nxtsct(&nstr, &sect)) {
	if (!player->owner)
	    continue;
	pr("%s at %s ", dchr[sect.sct_type].d_name,
	   xyas(nstr.x, nstr.y, player->cnum));
	if ((sect.sct_dist_x != sect.sct_x) ||
	    (sect.sct_dist_y != sect.sct_y)) {
	    getsect(sect.sct_dist_x, sect.sct_dist_y, &tsect);
	    if (tsect.sct_own != player->cnum)
		pr("distributes to %s, not owned by you.\n",
		   xyas(tsect.sct_x, tsect.sct_y, player->cnum));
	    else
		pr("distributes to %s. \n",
		   xyas(tsect.sct_x, tsect.sct_y, player->cnum));
	} else
	    pr("has no dist sector. \n");
	p = getstarg(player->argp[2], "Distribution sector? ", buf);
	if (p && (*p == 0))
	    continue;

	if (!check_sect_ok(&sect))
	    continue;

	if (p && (*p != '.') && (*p != 'h') && (!sarg_xy(p, &dstx, &dsty)))
	    return RET_SYN;

	if (p && ((*p == '.') || (*p == 'h'))) {
	    dstx = sect.sct_x;
	    dsty = sect.sct_y;
	}

	if (!getsect(dstx, dsty, &dsect)) {
	    pr("Bad sector.\n");
	    return RET_FAIL;
	}

	if (dsect.sct_own != player->cnum)
	    pr("Warning: you don't own %s!\n",
	       xyas(dsect.sct_x, dsect.sct_y, player->cnum));

	path = BestDistPath(buf, &sect, &dsect, &move_cost, MOB_ROAD);

	if (!path) {
	    pr("No owned path from %s to %s.\n",
	       xyas(dsect.sct_x, dsect.sct_y, player->cnum),
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	    continue;
	}

	if ((dsect.sct_x == sect.sct_x) && (dsect.sct_y == sect.sct_y)) {
	    pr("Distribution from and to %s %s terminated\n",
	       dchr[sect.sct_type].d_name,
	       xyas(sect.sct_x, sect.sct_y, player->cnum));
	} else {
	    pr("%s %s now distributes to %s (cost %1.3f)\n",
	       dchr[sect.sct_type].d_name,
	       xyas(sect.sct_x, sect.sct_y, player->cnum),
	       xyas(dsect.sct_x, dsect.sct_y, player->cnum), move_cost);
	}
	pr("\n");
	/* Only change and write out if we are really changing where it
	   distributes to.  Otherwise, it's a waste of time (since nothing
	   changed.) */
	if ((sect.sct_dist_x != dsect.sct_x) ||
	    (sect.sct_dist_y != dsect.sct_y)) {
	    sect.sct_dist_x = dsect.sct_x;
	    sect.sct_dist_y = dsect.sct_y;
	    putsect(&sect);
	}
    }
    return RET_OK;
}
