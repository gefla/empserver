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
 *  orig.c: Move your origin
 * 
 *  Known contributors to this file:
 *     Shelley Louie, 1988
 */

#include "misc.h"
#include "player.h"
#include "sect.h"
#include "xy.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

int
orig(void)
{
    struct sctstr sect;
    s_char *p;
    coord x, y;
    s_char buf[1024];
    struct natstr *np;

    if ((p =
	 getstarg(player->argp[1], "New origin location : ", buf)) == 0) {
	return RET_SYN;
    }
    if (!sarg_xy(p, &x, &y))
	return RET_SYN;
    if (!getsect(x, y, &sect))
	return RET_SYN;
    pr("Origin at %s (old system) is now at 0,0 (new system).\n",
       xyas(sect.sct_x, sect.sct_y, player->cnum));
    np = getnatp(player->cnum);
    np->nat_xorg = sect.sct_x;
    np->nat_yorg = sect.sct_y;
    putnat(np);
    return RET_OK;
}
