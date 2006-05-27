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
 *  wing.c: Add planes to a wing
 * 
 *  Known contributors to this file:
 *  
 */

#include <config.h>

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "plane.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

int
wing(void)
{

    struct plnstr plane;
    int count;
    char *cp;
    char c;
    struct nstr_item nstr;
    char buf[1024];

    if (!(cp = getstarg(player->argp[1], "wing? ", buf)))
	return RET_SYN;
    c = *cp;
    if (!isalpha(c) && c != '~') {
	pr("Specify wing, (1 alpha char or '~')\n");
	return RET_SYN;
    }
    if (c == '~')
	c = 0;
    if (!snxtitem(&nstr, EF_PLANE, player->argp[2]))
	return RET_SYN;
    count = 0;
    while (nxtitem(&nstr, &plane)) {
	if (plane.pln_own != player->cnum)
	    continue;
	if (plane.pln_wing == c)
	    continue;
	plane.pln_wing = c;
	putplane(plane.pln_uid, &plane);
	count++;
    }
    pr("%d plane%s added to wing `%c'\n", count, splur(count), c);
    return RET_OK;
}
