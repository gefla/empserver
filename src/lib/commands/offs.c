/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
 *  related information and legal notices. It is expected that any future
 *  projects/authors will amend these files as needed.
 *
 *  ---
 *
 *  offs.c: Diety set mapping offset.
 * 
 *  Known contributors to this file:
 *     
 */

#include <ctype.h>
#include <string.h>
#include "misc.h"
#include "player.h"
#include "file.h"
#include "nat.h"
#include "commands.h"

int
offs(void)
{
    register int i;
    struct natstr *natp;
    coord xorg, yorg;
    coord dx, dy;
    s_char *cp;
    struct natstr *np;
    s_char buf[1024];

    natp = getnatp(player->cnum);
    xorg = natp->nat_xorg;
    yorg = natp->nat_yorg;
    if (!(cp = getstarg(player->argp[1], "sector or nation? ", buf)))
	return RET_SYN;
    if (strchr(cp, ',')) {	/* x, y pair for offset */
	if (!sarg_xy(cp, &dx, &dy)) {
	    pr("Bad sector designation.\n");
	    return RET_SYN;
	}
	xorg = dx;
	yorg = dy;
    } else {
	/* actually nation name */
	if ((i = natarg(cp, (s_char *)0)) < 0)
	    return RET_SYN;
	np = getnatp(i);
	xorg = np->nat_xorg;
	yorg = np->nat_yorg;
    }
    np = getnatp(player->cnum);
    /* NOTE: it's OK to use %d,%d here, because we want abs coords */
    pr("Old offset was %d,%d\n", np->nat_xorg, np->nat_yorg);
    np->nat_xorg = xorg;
    np->nat_yorg = yorg;
    putnat(np);
    pr("New offset was %d,%d\n", xorg, yorg);
    return RET_OK;
}
