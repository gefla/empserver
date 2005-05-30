/*
 *  Empire - A multi-player, client/server Internet based war game.
 *  Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
 *  army.c: Add units to an army
 * 
 *  Known contributors to this file:
 *     
 */

#include <ctype.h>
#include "misc.h"
#include "player.h"
#include "land.h"
#include "nsc.h"
#include "file.h"
#include "commands.h"

int
army(void)
{
    struct lndstr land;
    int count;
    s_char *cp;
    s_char c;
    struct nstr_item nstr;
    struct nstr_item ni;
    struct lndstr land2;
    s_char buf[1024];

    cp = getstarg(player->argp[1], "army? ", buf);
    if (cp == 0)
	return RET_SYN;
    c = *cp;
    if (!isalpha(c) && c != '~') {
	pr("Specify army, (1 alpha char or '~')\n");
	return RET_SYN;
    }
    if (c == '~')
	c = ' ';
    if (!snxtitem(&nstr, EF_LAND, player->argp[2]))
	return RET_SYN;
    count = 0;
    while (nxtitem(&nstr, &land)) {
	if (!player->owner)
	    continue;
	if (land.lnd_army == c)
	    continue;
	land.lnd_rflags &= ~RET_GROUP;
	snxtitem_group(&ni, EF_LAND, c);
	while (nxtitem(&ni, &land2)) {
	    if ((land2.lnd_rflags & RET_GROUP) == 0)
		continue;
	    if (land2.lnd_x == land.lnd_x && land2.lnd_y == land.lnd_y) {
		memcpy(land.lnd_rpath, land2.lnd_rpath,
		       sizeof(land.lnd_rpath));
		land.lnd_rflags = land2.lnd_rflags;
		break;
	    }
	}
	land.lnd_army = c;
	putland(land.lnd_uid, &land);
	count++;
    }
    pr("%d unit%s added to army `%c'\n", count, splur(count), c);
    return RET_OK;
}
