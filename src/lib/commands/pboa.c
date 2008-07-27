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
 *  pboard.c: Capture an enemy plane
 *
 *  Known contributors to this file:
 *     John Yockey, 2001
 */

#include <config.h>

#include "commands.h"
#include "plane.h"

int
pboa(void)
{
    struct sctstr sect;
    struct nstr_item np;
    struct plnstr plane;

    if (!snxtitem(&np, EF_PLANE, player->argp[1], NULL))
	return RET_SYN;
    while (nxtitem(&np, &plane)) {
	getsect(plane.pln_x, plane.pln_y, &sect);
	if (sect.sct_own != player->cnum)
	    continue;
	takeover_plane(&plane, player->cnum);
    }
    return RET_OK;
}
