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
 *  wing.c: Add planes to a wing
 * 
 *  Known contributors to this file:
 *  
 */

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "var.h"
#include "plane.h"
#include "xy.h"
#include "nsc.h"
#include "nat.h"
#include "file.h"
#include "commands.h"

int
wing(void)
{

    struct plnstr plane;
    register int count;
    s_char *cp;
    s_char c;
    struct nstr_item nstr;
    s_char buf[1024];

    if (!(cp = getstarg(player->argp[1], "wing? ", buf)))
	return RET_SYN;
    c = *cp;
    if (!isalpha(c) && c != '~') {
	pr("Specify wing, (1 alpha char or '~')\n");
	return RET_SYN;
    }
    if (c == '~')
	c = ' ';
    if (!snxtitem(&nstr, EF_PLANE, player->argp[2]))
	return RET_SYN;
    for (count = 0; nxtitem(&nstr, (s_char *)&plane); count++) {
	if (plane.pln_own != player->cnum) {
	    count--;
	    continue;
	}
	plane.pln_wing = c;
	putplane(plane.pln_uid, &plane);
    }
    pr("%d plane%s added to wing `%c'\n", count, splur(count), c);
    return RET_OK;
}
