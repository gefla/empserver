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
 *  togg.c: Set player flags
 * 
 *  Known contributors to this file:
 *     Ken Stevens, 1995
 *     Steve McClure, 1996
 */

#include <config.h>

#include "commands.h"

int
togg(void)
{
    int flag = 0;
    int pos;
    char *name;
    struct natstr *np;

    np = getnatp(player->cnum);
    if (player->argp[1]) {
	switch (*player->argp[1]) {
	case 'i':
	    name = "inform";
	    flag = NF_INFORM;
	    break;
	case 'f':
	    name = "flash";
	    flag = NF_FLASH;
	    break;
	case 'b':
	    name = "beep";
	    flag = NF_BEEP;
	    break;
	case 'c':
	    name = "coastwatch";
	    flag = NF_COASTWATCH;
	    break;
	case 's':
	    name = "sonar";
	    flag = NF_SONAR;
	    break;
	case 't':
	    name = "techlists";
	    flag = NF_TECHLISTS;
	    break;
	default:
	    return RET_SYN;
	}
	if (player->argp[2])
	    if (!strcmp(player->argp[2], "on"))
		pos = 1;
	    else if (!strcmp(player->argp[2], "off"))
		pos = 0;
	    else
		return RET_SYN;
	else
	    pos = !(np->nat_flags & flag);
	if (pos)
	    np->nat_flags |= flag;
	else
	    np->nat_flags &= ~flag;
	putnat(np);
	pr("%s flag %s\n", name, pos ? "on" : "off");
    } else {
	if (np->nat_flags & NF_INFORM)
	    pr("inform flag on\n");
	else
	    pr("inform flag off\n");
	if (np->nat_flags & NF_FLASH)
	    pr("flash flag on\n");
	else
	    pr("flash flag off\n");
	if (np->nat_flags & NF_BEEP)
	    pr("beep flag on\n");
	else
	    pr("beep flag off\n");
	if (np->nat_flags & NF_COASTWATCH)
	    pr("coastwatch flag on\n");
	else
	    pr("coastwatch flag off\n");
	if (np->nat_flags & NF_SONAR)
	    pr("sonar flag on\n");
	else
	    pr("sonar flag off\n");
	if (np->nat_flags & NF_TECHLISTS)
	    pr("techlists flag on\n");
	else
	    pr("techlists flag off\n");
    }

    return RET_OK;
}
