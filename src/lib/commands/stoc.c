/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2013, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  stoc.c: Add nukes to a stockpile
 *
 *  Known contributors to this file:
 *     Markus Armbruster, 2013
 */

#include <config.h>

#include <ctype.h>
#include "commands.h"
#include "nuke.h"

int
stoc(void)
{
    struct nukstr nuke;
    int count;
    char *cp;
    char c;
    struct nstr_item nstr;
    char buf[1024];

    cp = getstarg(player->argp[1], "stockpile? ", buf);
    if (!cp)
	return RET_SYN;
    c = *cp;
    if (!isalpha(c) && c != '~') {
	pr("Specify stockpile, (1 alpha char or '~')\n");
	return RET_SYN;
    }
    if (c == '~')
	c = 0;
    if (!snxtitem(&nstr, EF_NUKE, player->argp[2], NULL))
	return RET_SYN;
    count = 0;
    while (nxtitem(&nstr, &nuke)) {
	if (!player->owner)
	    continue;
	if (nuke.nuk_stockpile == c)
	    continue;
	nuke.nuk_stockpile = c;
	putnuke(nuke.nuk_uid, &nuke);
	count++;
    }
    pr("%d nuke%s added to stockpile `%1.1s'\n", count, splur(count), &c);
    return RET_OK;
}
