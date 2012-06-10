/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2012, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  orig.c: Move your origin
 *
 *  Known contributors to this file:
 *     Shelley Louie, 1988
 *     Markus Armbruster, 2006-2011
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"

int
orig(void)
{
    char *p;
    coord x, y;
    char buf[1024];
    int cnum;
    struct natstr *np;

    p = getstarg(player->argp[1], "New origin (sector or country) : ", buf);
    if (!p || !*p)
	return RET_SYN;
    if (!isalpha(*p) && strchr(p, ',')) {
	/* sector */
	if (!sarg_xy(p, &x, &y)) {
	    pr("Bad sector designation.\n");
	    return RET_SYN;
	}
    } else if (*p == '~') {
	/* reset */
	if (!player->god) {
	    pr("Only deities can reset their origin.\n");
	    return RET_FAIL;
	}
	x = y = 0;
    } else {
	/* country */
	cnum = natarg(p, NULL);
	if (!(np = getnatp(cnum)))
	    return RET_SYN;
	if (!player->god && relations_with(cnum, player->cnum) != ALLIED) {
	    pr("Country %s is not allied with you!\n", np->nat_cnam);
	    return RET_FAIL;
	}
	x = np->nat_xorg;
	y = np->nat_yorg;
    }
    pr("Origin at %s (old system) is now at 0,0 (new system).\n",
       xyas(x, y, player->cnum));
    np = getnatp(player->cnum);
    np->nat_xorg = x;
    np->nat_yorg = y;
    putnat(np);
    return RET_OK;
}
